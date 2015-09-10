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
 * // import module, load dataset, create model, evaluate model
 */
/**
* SVC constructor parameters
* @typedef {Object} svcParam
* @property  {number} [svcParam.c=1.0] - Cost parameter. Increasing the parameter forces the model to fit the training data more accurately (setting it too large may lead to overfitting) .
* @property  {number} [svcParam.j=1.0] - Unbalance parameter. Increasing it gives more weight to the positive examples (getting a better fit on the positive training examples gets a higher priority). Setting c=n is like adding n-1 copies of the positive training examples to the data set.
* @property  {number} [svcParam.batchSize=1000] - Number of examples used in the subgradient estimation. Higher number of samples slows down the algorithm, but makes the local steps more accurate.
* @property  {number} [svcParam.maxIterations=10000] - Maximum number of iterations.
* @property  {number} [svcParam.maxTime=1] - Maximum runtime in seconds.
* @property  {number} [svcParam.minDiff=1e-6] - Stopping criterion tolerance.
* @property  {boolean} [svcParam.verbose=false] - Toggle verbose output in the console.
*/
/**
* SVC
* @classdesc Support Vector Machine Classifier. Implements a soft margin linear support vector classifier using the PEGASOS algorithm, see: {@link http://ttic.uchicago.edu/~nati/Publications/PegasosMPB.pdf Pegasos: Primal Estimated sub-GrAdient SOlver for SVM}.
* @class
* @param {module:fs.FIn | module:analytics~svcParam} [arg] - File input stream (loads the model from disk) or constructor parameters svcParam.
* @example
* // import modules
* var la = require('qminer').la;
* var analytics = require('qminer').analytics;
* // CLASSIFICATION WITH SVC
* // set up fake train and test data
* // four training examples with number of features = 2
* var featureMatrix = new la.Matrix({ rows: 2, cols: 4, random: true });
* // classification targets for four examples
* var targets = new la.Vector([-1, -1, 1, 1]);
* // set up the classification model
* var SVC = new analytics.SVC({ verbose: true });
* // train classifier
* SVC.fit(featureMatrix, targets);
* // set up a fake test vector
* var test = new la.Vector([1.1, -0.5]);
* // predict the target value
* var prediction = SVC.predict(test);
*/
 exports.SVC = function(arg) { return Object.create(require('qminer').analytics.SVC.prototype); };
/**
	* Returns the SVC parameters.
	* @returns {module:analytics~svcParam} Parameters of the classifier model.
	* @example
	* // import analytics module
	* var analytics = require('qminer').analytics;
	* // create a new SVC model with json
	* var SVC = new analytics.SVC({ c: 5, j: 10, batchSize: 2000, maxIterations: 12000, maxTime: 2, minDiff: 1e-10, verbose: true });
	* // get the parameters of the SVC model
	* // returns { algorithm: 'SGD' c: 5, j: 10, batchSize: 2000, maxIterations: 12000, maxTime: 2, minDiff: 1e-10, verbose: true }
	* var json = SVC.getParams(); 
	*/
 exports.SVC.prototype.getParams = function() { return { algorithm: '', c: 0, j: 0, batchSize: 0, maxIterations: 0, maxTime: 0, minDiff: 0, verbose: true } };
/**
	* Sets the SVC parameters.
	* @param {module:analytics~svcParam} param - Classifier training parameters.
	* @returns {module:analytics.SVC} Self.
	* @example
	* // import analytics module
	* var analytics = require('qminer').analytics;
	* // create a default SVC model
	* var SVC = new analytics.SVC();
	* // change the parameters of the SVC with the json { j: 5, maxIterations: 12000, minDIff: 1e-10 }
	* SVC.setParams({ j: 5, maxIterations: 12000, minDiff: 1e-10 }); // returns self
	*/
 exports.SVC.prototype.setParams = function(param) { return Object.create(require('qminer').analytics.SVC.prototype); };
/**	
	* Gets the vector of coefficients of the linear model.
	* @returns {module:la.Vector} weights - Vector of coefficients of the linear model.
	* @example 
	* // import the analytics and la modules
	* var analytics = require('qminer').analytics;
	* var la = require('qminer').la;
	* // create a new SVC object
	* var SVC = new analytics.SVC();
	* // create the matrix containing the input features and the input vector for each matrix.
	* var matrix = new la.Matrix([[1, 0, -1, 0], [0, 1, 0, -1]]);
	* var vec = new la.Vector([1, 1, -1, -1]);
	* // fit the model
	* SVC.fit(matrix, vec);
	* // get the weights
	* var weights = SVC.weights; // returns the coefficients of the normal vector of the hyperplane gained from the model: [1, 1]
	*/
 exports.SVC.prototype.weights = Object.create(require('qminer').la.Vector.prototype);
/**
	* Saves model to output file stream.
	* @param {module:fs.FOut} fout - Output stream.
	* @returns {module:fs.FOut} The Output stream.
	*/
 exports.SVC.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); }
/**
    * Sends vector through the model and returns the distance to the decision boundery.
    * @param {module:la.Vector | module:la.SparseVector | module:la.Matrix | module:la.SparseMatrix} X - Input feature vector or matrix with feature vectors as columns.
    * @returns {number | module:la.Vector} Distance:
	* <br>1. Real number, if input is {@link module:la.Vector} or {@link module:la.SparseVector}.
	* <br>2. {@link module:la.Vector}, if input is {@link module:la.Matrix} or {@link module:la.SparseMatrix}.
	* <br>Sign of the number corresponds to the class and the magnitude corresponds to the distance from the margin (certainty).
    * @example
	* // import the analytics and la modules
	* var analytics = require('qminer').analytics;
	* var la = require('qminer').la;
	* // create a new SVC object
	* var SVC = new analytics.SVC();
	* // create the matrix containing the input features and the input vector for each matrix
	* var matrix = new la.Matrix([[1, 0], [0, -1]]);
	* var vec = new la.Vector([1, -1]);
	* // fit the model
	* SVC.fit(matrix, vec);
	* // create the vector you want to get the distance from the model
	* var vec2 = new la.Vector([2, 3]);
	* // use the decisionFunction to get the distance of vec2 from the model
	* var distance = SVC.decisionFunction(vec2); // returns something close to 5
	*/
 exports.SVC.prototype.decisionFunction = function(X) { return (X instanceof require('qminer').la.Vector | X instanceof require('qminer').la.SparseVector) ? 0 : Object.create(require('qminer').la.Vector.prototype); }
/**
	* Sends vector through the model and returns the prediction as a real number.
    * @param {module:la.Vector | module:la.SparseVector | module:la.Matrix | module:la.SparseMatrix} X - Input feature vector or matrix with feature vectors as columns.
    * @returns {number | module:la.Vector} Prediction:
	* <br>1. Real number, if input is {@link module:la.Vector} or {@link module:la.SparseVector}.
	* <br>2. {@link module:la.Vector}, if input is {@link module:la.Matrix} or {@link module:la.SparseMatrix}.
	* <br>1 for positive class and -1 for negative.
	* @example
	* // import the analytics and la modules
	* var analytics = require('qminer').analytics;
	* var la = require('qminer').la;
	* // create a new SVC object
	* var SVC = new analytics.SVC();
	* // create the matrix containing the input features and the input vector for each matrix
	* var matrix = new la.Matrix([[1, 0, -1, 0], [0, 1, 0, -1]]);
	* var vec = new la.Vector([1, 1, -1, -1]);
	* // fit the model
	* SVC.fit(matrix, vec);
	* // create a vector you want to predict 
	* var vec2 = new la.Vector([3, 5]);
	* // predict the vector
	* var prediction = SVC.predict(vec2); // returns 1
	*/
 exports.SVC.prototype.predict = function(X) { return (X instanceof require('qminer').la.Vector | X instanceof require('qminer').la.SparseVector) ? 0 : Object.create(require('qminer').la.Vector.prototype); }
/**
	* Fits a SVM classification model, given column examples in a matrix and vector of targets.
	* @param {module:la.Matrix | module:la.SparseMatrix} X - Input feature matrix where columns correspond to feature vectors.
	* @param {module:la.Vector} y - Input vector of targets, one for each column of X.
	* @returns {module:analytics.SVC} Self.
	* @example
	* // import the analytics and la modules
	* var analytics = require('qminer').analytics;
	* var la = require('qminer').la;
	* // create a new SVC object
	* var SVC = new analytics.SVC();
	* // create the matrix containing the input features and the input vector for each matrix.
	* var matrix = new la.Matrix([[1, 0, -1, 0], [0, 1, 0, -1]]);
	* var vec = new la.Vector([1, 1, -1, -1]);
	* // fit the model
	* SVC.fit(matrix, vec); // creates a model, where the hyperplane has the normal semi-equal to [1, 1]
	*/
 exports.SVC.prototype.fit = function(X, y) { return Object.create(require('qminer').analytics.SVC.prototype); }
/**
* SVR constructor parameters
* @typedef {Object} svrParam
* @property  {number} [svrParam.c=1.0] - Cost parameter. Increasing the parameter forces the model to fit the training data more accurately (setting it too large may lead to overfitting).
* @property  {number} [svrParam.eps=1e-1] - Epsilon insensitive loss parameter. Larger values result in fewer support vectors (smaller model complexity).
* @property  {number} [svrParam.batchSize=1000] - Number of examples used in the subgradient estimation. Higher number of samples slows down the algorithm, but makes the local steps more accurate.
* @property  {number} [svrParam.maxIterations=10000] - Maximum number of iterations.
* @property  {number} [svrParam.maxTime=1.0] - Maximum runtime in seconds.
* @property  {number} [svrParam.minDiff=1e-6] - Stopping criterion tolerance.
* @property  {boolean} [svrParam.verbose=false] - Toggle verbose output in the console.
*/
/**
* SVR
* @classdesc Support Vector Machine Regression. Implements a soft margin linear support vector regression using the PEGASOS algorithm with epsilon insensitive loss, see: {@link http://ttic.uchicago.edu/~nati/Publications/PegasosMPB.pdf Pegasos: Primal Estimated sub-GrAdient SOlver for SVM}.
* @class
* @param {module:fs.FIn | module:analytics~svrParam} [arg] - File input stream (loads the model from disk) or constructor parameters svcParam.
* @example
* // import module
* var analytics = require('qminer').analytics;
* var la = require('qminer').la;
* // REGRESSION WITH SVR
* // Set up fake train and test data.
* // Four training examples with, number of features = 2
* var featureMatrix = new la.Matrix({ rows: 2, cols: 4, random: true });
* // Regression targets for four examples
* var targets = new la.Vector([1.1, -2, 3, 4.2]);
* // Set up the regression model
* var SVR = new analytics.SVR({ verbose: true });
* // Train regression
* SVR.fit(featureMatrix, targets);
* // Set up a fake test vector
* var test = new la.Vector([1.1, -0.8]);
* // Predict the target value
* var prediction = SVR.predict(test);
*/
 exports.SVR = function(arg) { return Object.create(require('qminer').analytics.SVR.prototype); };
/**
	* Returns the SVR parameters.
	* @returns {module:analytics~svrParam} Parameters of the regression model.
	* @example
	* // import analytics module
	* var analytics = require('qminer').analytics;
	* // create a new SVR object
	* var SVR = new analytics.SVR({ c: 10, eps: 1e-10, maxTime: 12000, verbose: true });
	* // get the parameters of SVR
	* var params = SVR.getParams();
	*/
 exports.SVR.prototype.getParams = function() { return { c: 0, eps: 0, batchSize: 0, maxIterations: 0, maxTime: 0, minDiff: 0, verbose: true } };
/**
	* Sets the SVR parameters.
	* @param {module:analytics~svrParam} param - Regression training parameters.
	* @returns {module:analytics.SVR} Self.
	* @example
	* // import analytics module
	* var analytics = require('qminer').analytics;
	* // create a new SVR object
	* var SVR = new analytics.SVR();
	* // set the parameters of the SVR object
	* SVR.setParams({ c: 10, maxTime: 12000 });
	*/
 exports.SVR.prototype.setParams = function(param) { return Object.create(require('qminer').analytics.SVR.prototype); };
/**
	* The vector of coefficients of the linear model.
	* @returns {module:la.Vector} weights - Vector of coefficients of the linear model.
	*/
 exports.SVR.prototype.weights = Object.create(require('qminer').la.Vector.prototype);
/**
	* Saves model to output file stream.
	* @param {module:fs.FOut} fout - Output stream.
	* @returns {module:fs.FOut} Output stream.
	*/
 exports.SVR.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); }
/**
     * Sends vector through the model and returns the scalar product as a real number.
     * @param {module:la.Vector | module:la.SparseVector | module:la.Matrix | module:la.SparseMatrix} X - Input feature vector or matrix with feature vectors as columns.
     * @returns {number | module:la.Vector} Distance:
	 * <br>1. Real number if input is {@link module:la.Vector} or {@link module:la.SparseVector}.
	 * <br>2. {@link module:la.Vector}, if input is {@link module:la.Matrix} or {@link module:la.SparseMatrix}.
	 * @example
	 * // import the modules
	 * var analytics = require('qminer').analytics;
	 * var la = require('qminer').la;
	 * // create a new SVR object
	 * var SVR = new analytics.SVR({ c: 10 });
	 * // create a matrix and vector for the model
	 * var matrix = new la.Matrix([[1, -1], [1, 1]]);
	 * var vector = new la.Vector([1, 1]);
	 * // create the model by fitting the values
	 * SVR.fit(matrix, vector);
	 * // get the distance between the model and the given vector
	 * var vec2 = new la.Vector([-5, 1]);
	 * var distance = SVR.decisionFunction(vec2);
     */
 exports.SVR.prototype.decisionFunction = function(X) { return (X instanceof require('qminer').la.Vector | X instanceof require('qminer').la.SparseVector) ? 0 : Object.create(require('qminer').la.Vector.prototype); }
/**
	* Sends vector through the model and returns the prediction as a real number.
    * @param {module:la.Vector | module:la.SparseVector | module:la.Matrix | module:la.SparseMatrix} X - Input feature vector or matrix with feature vectors as columns.
    * @returns {number | module:la.Vector} Prediction:
	* <br>1. Real number, if input is {@link module:la.Vector} or {@link module:la.SparseVector}.
	* <br>2. {@link module:la.Vector}, if input is {@link module:la.Matrix} or {@link module:la.SparseMatrix}.
	* @example
	* // import the modules
	* var analytics = require('qminer').analytics;
	* var la = require('qminer').la;
	* // create a new SVR object
	* var SVR = new analytics.SVR({ c: 10 });
	* // create a matrix and vector for the model
	* var matrix = new la.Matrix([[1, -1], [1, 1]]);
	* var vector = new la.Vector([1, 1]);
	* // create the model by fitting the values
	* SVR.fit(matrix, vector);
	* // predict the value of the given vector
	* var vec2 = new la.Vector([-5, 1]);
	* var prediction = SVR.predict(vec2);
	*/
 exports.SVR.prototype.predict = function(X) { return (X instanceof require('qminer').la.Vector | X instanceof require('qminer').la.SparseVector) ? 0 : Object.create(require('qminer').la.Vector.prototype); }
/**
	* fits an SVM regression model, given column examples in a matrix and vector of targets
	* @param {module:la.Matrix | module:la.SparseMatrix} X - Input feature matrix where columns correspond to feature vectors.
	* @param {module:la.Vector} y - Input vector of targets, one for each column of X.
	* @returns {module:analytics.SVR} Self.
	* @example
	* // import the modules
	* var analytics = require('qminer').analytics;
	* var la = require('qminer').la;
	* // create a new SVR object
	* var SVR = new analytics.SVR({ c: 10 });
	* // create a matrix and vector for the model
	* var matrix = new la.Matrix([[1, -1], [1, 1]]);
	* var vector = new la.Vector([1, 1]);
	* // create the model by fitting the values
	* SVR.fit(matrix, vector);
	*/
 exports.SVR.prototype.fit = function(X, y) { return Object.create(require('qminer').analytics.SVR.prototype); }
/**
* @typedef {Object} ridgeRegParam
* The Ridge Regression constructor parameter.
* @param {number} [gamma=1.0] - The gamma value.
*/
/**
 * Ridge regression. Minimizes: ||A' x - b||^2 + ||gamma x||^2
 *
 * Uses {@link http://en.wikipedia.org/wiki/Tikhonov_regularization Tikhonov regularization}.
 *
 * @class
 * @param {(module:analytics~ridgeRegParam|module:fs.FIn)} [arg] - Loads a model from input stream, or creates a new model by setting gamma=arg from a Json object.
 * Empty constructor sets gamma to zero.
 * @example
 * // import modules
 * la = require('qminer').la;
 * analytics = require('qminer').analytics;
 * // create a new model with gamma = 1.0
 * var regmod = new analytics.RidgeReg(1.0);
 * // generate a random feature matrix
 * var A = la.randn(10,100);
 * // generate a random model
 * var w = la.randn(10);
 * // generate noise
 * var n = la.randn(100).multiply(0.01);
 * // generate responses (model'*data + noise)
 * var b = A.transpose().multiply(w).plus(n);
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
	* Gets the parameters.
	* @returns {Object} The Json object containing the parameters.
	* @example
	* // import analytics module
	* var analytics = require('qminer').analytics;
	* // create a new Ridge Regression object
	* var regmod = new analytics.RidgeReg({ gamma: 5 });
	* // get the parameters
	* // returns a json object { gamma: 5 }
	* var param = regmod.getParams();
	*/
 exports.RidgeReg.prototype.getParams = function () { return { gamma: 0.0 } }
/**
	* Set the parameters.
	* @param {(number|Object)} gamma - The new parameter for the model, given as a number or as a json object.
	* @returns {module:analytics.RidgeReg} Self. The parameter is set to gamma.
	* @example
	* // import analytics module
	* var analytics = require('qminer').analytics;
	* // create a new Ridge Regression object
	* var regmod = new analytics.RidgeReg({ gamma: 5 });
	* // set the parameters of the object
	* var param = regmod.setParams({ gamma: 10 });
	*/
 exports.RidgeReg.prototype.setParams = function (gamma) { return Object.create(require('qminer').analytics.RidgeReg.prototype); }
/**
     * Fits a column matrix of feature vectors X onto the response variable y.
     *
     * @param {module:la.Matrix} X - Column matrix which stores the feature vectors.
     * @param {module:la.Vector} y - Response variable.
     * @returns {module:analytics.RidgeReg} Self. The model is fitted by X and y.
	 * @example
	 * // import modules
	 * var analytics = require('qminer').analytics;
	 * var la = require('qminer').la;
	 * // create a new Ridge Regression object
	 * var regmod = new analytics.RidgeReg();
	 * // create the test matrix and vector
	 * var X = new la.Matrix([[1, 2], [1, -1]]);
	 * var y = new la.Vector([3, 3]);
	 * // fit the model with X and y
	 * // the weights of the model are 2, 1
	 * regmod.fit(X, y);
     */
 exports.RidgeReg.prototype.fit = function(X, y) { return Object.create(require('qminer').analytics.RidgeReg.prototype); }
/**
     * Returns the expected response for the provided feature vector.
     *
     * @param {module:la.Vector} x - Feature vector.
     * @returns {number} Predicted response.
	 * @example
	 * // import modules
	 * var analytics = require('qminer').analytics;
	 * var la = require('qminer').la;
	 * // create a new Ridge Regression object
	 * var regmod = new analytics.RidgeReg();
	 * // create the test matrix and vector
	 * var X = new la.Matrix([[1, 2], [1, -1]]);
	 * var y = new la.Vector([3, 3]);
	 * // fit the model with X and y
	 * regmod.fit(X, y);
	 * // create a new vector for the prediction
	 * var vec = new la.Vector([3, 4]);
	 * // create the prediction
	 * // returns the value 10
	 * var prediction = regmod.decisionFunction(vec);
     */
 exports.RidgeReg.prototype.decisionFunction = function(X) { return 0.0; }
/**
     * Returns the expected response for the provided feature vector.
     *
     * @param {module:la.Vector} x - Feature vector.
     * @returns {number} Predicted response.
	 * @example
	 * // import modules
	 * var analytics = require('qminer').analytics;
	 * var la = require('qminer').la;
	 * // create a new Ridge Regression object
	 * var regmod = new analytics.RidgeReg();
	 * // create the test matrix and vector
	 * var X = new la.Matrix([[1, 2], [1, -1]]);
	 * var y = new la.Vector([3, 3]);
	 * // fit the model with X and y
	 * regmod.fit(X, y);
	 * // create a new vector for the prediction
	 * var vec = new la.Vector([3, 4]);
	 * // create the prediction
	 * // returns the value 10
	 * var prediction = regmod.predict(vec);
     */
 exports.RidgeReg.prototype.predict = function(X) { return 0.0; }
/**
     * @property {module:la.Vector} weights - Vector of coefficients for linear regression.
     */
 exports.RidgeReg.prototype.weights = Object.create(require('qminer').la.Vector.prototype);
/**
     * Saves the model into the output stream.
     *
     * @param {module:fs.FOut} fout - Output stream.
	 * @returns {module:fs.FOut} THe output stream fout.
     */
 exports.RidgeReg.prototype.save = function(fout) { Object.create(require('qminer').fs.FOut.prototype); };
/**
 * Sigmoid function (y = 1/[1 + exp[-A*x + B]]) fitted on decision function to mimic.
 *
 * @class
 * @param {(null|module:fs.FIn)} [arg] - Loads a model from input stream, or creates a new model.
 * @example
 * // import modules
 * la = require('qminer').la;
 * analytics = require('qminer').analytics;
 * // create a new model
 * var sigmoid = new analytics.Sigmoid();
 * // generate a random predictions
 * var x = new la.Vector([0.5, 2.3, -0.1, 0.5, -7.3, 1.2]);
 * // generate a random labels
 * var y = new la.Vector([1, 1, -1, 1, -1, -1]);
 * // fit model
 * sigmoid.fit(x, y);
 * // get predictions
 * var pred1 = sigmoid.predict(1.2);
 * var pred2 = sigmoid.predict(-1.2);
 */
 exports.Sigmoid = function(arg) {};
/**
	* Get the parameters. It doesn't do anything, it's only for consistency for constructing pipeline.
	* @returns {Object} The Json object containing parameters.
	* @example
	* // import analytics module
	* var analytics = require('qminer').analytics;
	* // create the Sigmoid model
	* var s = new analytics.Sigmoid();
	* // get the parameters
	* // returns an empty Json object
	* var param = s.getParams();
	*/
 exports.Sigmoid.prototype.getParams = function () { return {}; }
/**
	* Sets the parameters. It doesn't do anything, it's only for consistency for constructing pipeline.
	* @param {Object} arg - Json object. 
	* @returns {module:analytics.Sigmoid} Self.
	* @example
	* // import analytics module
	* var analytics = require('qminer').analytics;
	* // create the Sigmoid model
	* var s = new analytics.Sigmoid();
	* // set the parameters 
	* // doesn't change the model
	* s.setParams({});
	*/
 exports.Sigmoid.prototype.setParams = function (arg) { return Object.create(require('qminer').analytics.Sigmoid.prototype); }
/**
	* Gets the model.
	* @returns {Object} The Json object containing the A and B values of the Sigmoid.
	* @example
	* // import analytics module
	* var analytics = require('qminer').analytics;
	* // create the Sigmoid model
	* var s = new analytics.Sigmoid();
	* // get the model parameters
	* // returns a Json object { A: 0, B: 0 }
	* var model = s.getModel();
	*/
 exports.Sigmoid.prototype.getModel = function () {return { A: 0, B: 0 }; }
/**
     * Fits a column matrix of feature vectors X onto the response variable y.
     *
     * @param {module:la.Vector} x - Predicted values (e.g., using analytics.SVR)
     * @param {module:la.Vector} y - Actual binary labels: 1 or -1.
     * @returns {module:analytics.Sigmoid} Self.
	 * @example
	 * // import modules
	 * var analytics = require('qminer').analytics;
	 * var la = require('qminer').la;
	 * // create the Sigmoid model
	 * var s = new analytics.Sigmoid();
	 * // create the predicted values and the binary labels
	 * var X = new la.Vector([-3, -2, -1, 1, 2, 3]);
	 * var y = new la.Vector([-1, -1, -1, 1, 1, 1]);
	 * // fit the model
	 * // changes the internal A and B values of the model 
	 * // (these values can be obtained with the getModel method)
	 * s.fit(X, y);
     */
 exports.Sigmoid.prototype.fit = function(X, y) { return Object.create(require('qminer').analytics.Sigmoid.prototype); }
/**
     * Returns the expected response for the provided feature vector.
     *
     * @param {(number|module:la.Vector)} x - Prediction score (or vector of them).
     * @returns {(number|module:la.Vector)} Normalized prediction score (or vector of them).
	 * @example
	 * // import modules
	 * var analytics = require('qminer').analytics;
	 * var la = require('qminer').la;
	 * // create the Sigmoid model
	 * var s = new analytics.Sigmoid();
	 * // create the predicted values and the binary labels
	 * var X = new la.Vector([-3, -2, -1, 1, 2, 3]);
	 * var y = new la.Vector([-1, -1, -1, 1, 1, 1]);
	 * // fit the model
	 * s.fit(X, y);
	 * // predict the probability of the value 0 on this model
	 * // returns 0.5
	 * var prediction = s.decisionFunction(0.5);
     */
 exports.Sigmoid.prototype.decisionFunction = function(x) { return (x instanceof la.Vector) ? Object.create(require('qminer').la.Vector.prototype) : 0.0; }
/**
     * Returns the expected response for the provided feature vector.
     *
     * @param {(number|module:la.Vector)} x - Prediction score (or vector of them).
     * @returns {(number|module:la.Vector)} Normalized prediction score (or vector of them).
	 * @example
	 * // import modules
	 * var analytics = require('qminer').analytics;
	 * var la = require('qminer').la;
	 * // create the Sigmoid model
	 * var s = new analytics.Sigmoid();
	 * // create the predicted values and the binary labels
	 * var X = new la.Vector([-3, -2, -1, 1, 2, 3]);
	 * var y = new la.Vector([-1, -1, -1, 1, 1, 1]);
	 * // fit the model
	 * s.fit(X, y);
	 * // predict the probability of the value 0 on this model
	 * // returns 0.5
	 * var prediction = s.predict(0.5);
     */
 exports.Sigmoid.prototype.predict = function(x) { return (x instanceof la.Vector) ? Object.create(require('qminer').la.Vector.prototype) : 0.0; }
/**
     * Saves the model into the output stream.
     *
     * @param {module:fs.FOut} fout - Output stream.
	 * @returns {module:fs.FOut} The output stream fout.
     */
 exports.Sigmoid.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); };
/**
* @typedef {Object} detectorParam
* A Json object used for the creation of the {@link module:analytics.NearestNeighborAD}.
* @param {number} [rate=0.05] - The expected fracton of emmited anomalies (0.05 -> 5% of cases will be classified as anomalies).
* @param {number} [windowSize=100] - Number of most recent instances kept in the model.
*/
/**
 * Nearest Neighbour Anomaly Detection 
 * @classdesc Anomaly detector that checks if the test point is too far from the nearest known point.
 * @class
 * @param {module:analytics~detectorParam} [detectorParam] - Constructor parameters.
 * @example
 * // import modules
 * var analytics = require('qminer').analytics;
 * var la = require('qminer').la;
 * // create a new NearestNeighborAD object
 * var neighbor = new analytics.NearestNeighborAD({ rate: 0.1 });
 * // create a sparse matrix 
 * var matrix = new la.SparseMatrix([[[0, 1], [1, 2]], [[0, -2], [1, 3]], [[0, 0], [1, 1]]]);
 * // fit the model with the matrix
 * neighbor.fit(matrix);
 * // create a new sparse vector
 * var vector = new la.SparseVector([[0, 4], [1, 0]]);
 * // predict if the vector is an anomaly or not
 * var prediction = neighbor.predict(vector);
 */
 exports.NearestNeighborAD = function(arg) { return Object.create(require('qminer').analytics.NearestNeighborAD.prototype); };
/**
	* Sets parameters.
	* @param {module:analytics~detectorParam} newParams - The Json object containing the new rate value.
	* @returns {module:analytics.NearestNeighborAD} Self. The parameters are updated with newParams.
	* @example
	* // import analytics module
	* var analytics = require('qminer').analytics;
	* // create a new NearestNeighborAD object
	* var neighbor = new analytics.NearestNeighborAD();
	* // set it's parameters to rate: 0.1
	* neighbor.setParams({ rate: 0.1 });
	*/
 exports.NearestNeighborAD.prototype.setParams = function (newParams) { return Object.create(require('qminer').analytics.NearestNeighborAD.prototype); }
/**
	* Returns parameters.
	* @returns {module:analytics~detectorParam} The Json object containing the rate value.
	* @example
	* // import analytics module
	* var analytics = require('qminer').analytics;
	* // create a new NearestNeighborAD object
	* var neighbor = new analytics.NearestNeighborAD();
	* // get the parameters of the object
	* // returns a json object { rate: 0.05 }
	* var params = neighbor.getParams();
	*/
 exports.NearestNeighborAD.prototype.getParams = function () { return { rate: 0.0, windowSize: 0.0 }; }
/**
     * Save model to provided output stream.
     * @param {module:fs.FOut} fout - The output stream.
     * @returns {module:fs.FOut} Provided output stream fout.
     */
 exports.NearestNeighborAD.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); }
/**
	* Returns the model.
	* @returns {Object} Json object whose keys are:
	* <br> 1. rate - The expected fraction of emmited anomalies.
	* <br> 2. thresh - Maximal squared distance to the nearest neighbor that is not anomalous.
	* @example
	* // import analytics module
	* var analytics = require('qminer').analytics;
	* // create a new NearestNeighborAD object
	* var neighbor = new analytics.NearestNeighborAD({ rate: 0.1 });
	* // get the model of the object
	* // returns a json object { rate: 0.1, window: 0 }
	* var model = neighbor.getModel();
	*/
 exports.NearestNeighborAD.prototype.getModel = function () { return { threshold: 0.0 }; }
/**
	* Adds a new point (or points) to the known points and recomputes the threshold.
	* @param {(module:la.SparseVector | module:la.SparseMatrix)} X - Test example (vector input) or column examples (matrix input).
	* @returns {module:analytics.NearestNeighborAD} Self. The model is updated.
	* @example
	* // import modules
	* var analytics = require('qminer').analytics;
	* var la = require('qminer').la;
	* // create a new NearestNeighborAD object
	* var neighbor = new analytics.NearestNeighborAD();
	* // create a new sparse matrix
	* var matrix = new la.SparseMatrix([[[0, 1], [1, 2]], [[0, -2], [1, 3]], [[0, 0], [1, 1]]]);
	* // fit the model with the matrix
	* neighbor.fit(matrix);
	* // create a new sparse vector
	* var vector = new la.SparseVector([[0, 2], [1, 5]]);
	* // update the model with the vector
	* neighbor.partialFit(vector);
	*/
 exports.NearestNeighborAD.prototype.partialFit = function(X) { return Object.create(require('qminer').NearestNeighborAD.prototype); }
/**
	* Analyzes the nearest neighbor distances and computes the detector threshold based on the rate parameter.
	* @param {module:la.SparseMatrix} A - Matrix whose columns correspond to known examples. Gets saved as it is part of
	* the model.
	* @returns {module:analytics.NearestNeighborAD} Self. The model is set by the matrix A.
	* @example
	* // import modules
	* var analytics = require('qminer').analytics;
	* var la = require('qminer').la;
	* // create a new NearestNeighborAD object
	* var neighbor = new analytics.NearestNeighborAD();
	* // create a new sparse matrix
	* var matrix = new la.SparseMatrix([[[0, 1], [1, 2]], [[0, -2], [1, 3]], [[0, 0], [1, 1]]]);
	* // fit the model with the matrix
	* neighbor.fit(matrix);
	*/
 exports.NearestNeighborAD.prototype.fit = function(A) { return Object.create(require('qminer').NearestNeighborAD.prototype); }
/**
     * Compares the point to the known points and returns distance to the nearest one.
     * @param {module:la.Vector} x - Test vector.
     * @returns {number} Distance to the nearest point.
	 * @example
	 * // import modules
	 * var analytics = require('qminer').analytics;
	 * var la = require('qminer').la;
	 * // create a new NearestNeighborAD object
	 * var neighbor = new analytics.NearestNeighborAD();
	 * // create a new sparse matrix
	 * var matrix = new la.SparseMatrix([[[0, 1], [1, 2]], [[0, -2], [1, 3]], [[0, 0], [1, 1]]]);
	 * // fit the model with the matrix
	 * neighbor.fit(matrix);
	 * // create a new sparse vector
	 * var vector = new la.SparseVector([[0, 4], [1, 0]]);
	 * // get the distance of the vector from the model
	 * var prediction = neighbor.decisionFunction(vector); // returns 1
	 */
 exports.NearestNeighborAD.prototype.decisionFunction = function(x) { return 0.0; }
/**
	* Compares the point to the known points and returns 1 if it's too far away (based on the precomputed threshold).
	* @param {module:la.SparseVector} x - Test vector.
	* @returns {number} Returns 1.0 if the vector x is an anomaly and 0.0 otherwise.
	* @example
	* // import modules
	* var analytics = require('qminer').analytics;
	* var la = require('qminer').la;
	* // create a new NearestNeighborAD object
	* var neighbor = new analytics.NearestNeighborAD();
	* // create a new sparse matrix
	* var matrix = new la.SparseMatrix([[[0, 1], [1, 2]], [[0, -2], [1, 3]], [[0, 0], [1, 1]]]);
	* // fit the model with the matrix
	* neighbor.fit(matrix);
	* // create a new sparse vector
	* var vector = new la.SparseVector([[0, 4], [1, 0]]);
	* // check if the vector is an anomaly
	* var prediction = neighbor.predict(vector); // returns 1
	*/
 exports.NearestNeighborAD.prototype.predict = function(x) { return 0.0; }
/**
* @typedef {Object} recLinearRegParam
* The constructor parameter for {@link module:analytics.RecLinReg}.
* @param {number} dim - The dimension of the model.
* @param {number} [regFact=1.0] - The regularization factor.
* @param {number} [forgetFact=1.0] - The forgetting factor.
*/
/**
* Recursive Linear Regression
* @classdesc Holds the Recursive Linear Regression model.
* @class
* @param {module:analytics~recLinearRegParam} param - The constructor parameter json object.
* @example
* // import analytics module
* var analytics = require('qminer').analytics;
* // create the recursive linear regression model holder
* var linreg = new analytics.RecLinReg({ dim: 10, regFact: 1.0, forgetFact: 1.0 });
*/
 exports.RecLinReg = function(param) { return Object.create(require('qminer').analytics.RecLinReg.prototype); }
/**
	* Creates a partial fit of the input.
	* @param {module:la.Vector} vec - The input vector.
	* @param {number} num - The target number for the vector.
	* @returns {module:analytics.RecLinReg} Self. The internal model is updated.
	* @example
	* // import modules
	* var analytics = require('qminer').analytics;
	* var la = require('qminer').la;
	* // create the Recursive Linear Regression model
	* var linreg = new analytics.RecLinReg({ dim: 3.0 });
	* // create a new dense vector
	* var vec = new la.Vector([1, 2, 3]);
	* // fit the model with the vector
	* linreg.partialFit(vec, 6);
	*/
 exports.RecLinReg.prototype.partialFit = function (vec, num) { return Object.create(require('qminer').analytics.RecLinReg.prototype); }
/**
	* Creates a fit of the input.
	* @param {module:la.Matrix} mat - The input matrix.
	* @param {module:la.Vector} vec - The target numbers, where the i-th number in vector is the target number for the i-th column of the matrix.
	* @returns {module:analytics.RecLinReg} Self. The internal model is updated.
	* @example
	* // import modules
	* var analytics = require('qminer').analytics;
	* var la = require('qminer').la;
	* // create the Recursive Linear Regression model
	* var linreg = new analytics.RecLinReg({ dim: 2.0 });
	* // create a new dense matrix and target vector
	* var mat = new la.Matrix([[1, 2, 3], [3, 4, 5]]);
	* var vec = new la.Vector([3, 5, -1]);
	* // fit the model with the matrix
	* linreg.fit(mat, vec);
	*/
 exports.RecLinReg.prototype.fit = function (mat, vec) { return Object.create(require('qminer').analytics.RecLinReg.prototype); }
/**
	* Puts the vector through the model and returns the prediction as a real number.
	* @param {module:la.Vector} vec - The vector needed to be predicted.
	* @returns {number} The prediction.
	* @example
	* // import modules
	* var analytics = require('qminer').analytics;
	* var la = require('qminer').la;
	* // create the Recursive Linear Regression model
	* var linreg = new analytics.RecLinReg({ dim: 2.0, recFact: 1e-10 });
	* // create a new dense matrix and target vector
	* var mat = new la.Matrix([[1, 2], [1, -1]]);
	* var vec = new la.Vector([3, 3]);
	* // fit the model with the matrix
	* linreg.fit(mat, vec);
	* // create the vector to be predicted
	* var pred = new la.Vector([1, 1]);
	* // predict the value of the vector
	* var prediction = linreg.predict(pred); // returns something close to 3.0
	*/
 exports.RecLinReg.prototype.predict = function (vec) { return 0.0 }  
/**
	* Sets the parameters of the model.
	* @param {module:analytics~recLinearRegParam} param - The new parameters of the model.
	* @returns {module:analytics.RecLinReg} Self. The parameters are updated. Any previous model is set to default.
	* @example
	* // import analytics module
	* var analytics = require('qminer').analytics;
	* // create a new Recursive Linear Regression model
	* var linreg = new analytics.RecLinReg({ dim: 10 });
	* // set the parameters of the model
	* linreg.setParams({ dim: 3, recFact: 1e2, forgetFact: 0.5 });
	*/
 exports.RecLinReg.prototype.setParams = function (param) { return Object.create(require('qminer').analytics.RecLinReg.prototype); }
/**
	* Returns the parameters.
	* @returns {module:analytics~recLinearRegParam} The parameters of the model.
	* @example
	* // import analytics module
	* var analytics = require('qminer').analytics;
	* // create a new Recursive Linear Regression model
	* var linreg = new analytics.RecLinReg({ dim: 10 });
	* // get the parameters of the model
	* var params = linreg.getParams(); // returns { dim: 10, recFact: 1.0, forgetFact: 1.0 }
	*/
 exports.RecLinReg.prototype.getParams = function () { return { dim: 0, regFact: 1.0, forgetFact: 1.0 }}
/**
	* Gives the weights of the model.
	*/
 exports.RecLinReg.prototype.weights = Object.create(require('qminer').la.Vector);
/**
	* Gets the dimensionality of the model.
	*/
 exports.RecLinReg.prototype.dim = 0;
/**
	* Save model to provided output stream.
	* @param {module:fs.FOut} fout - The output stream.
	* @returns {module:fs.FOut} Provided output stream fout.
	*/
 exports.RecLinReg.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); }
/**
* @typedef {Object} logisticRegParam
* The Json constructor parameters for {@link module:analytics.LogReg}.
* @property {number} [lambda=1] - The regularization parameter.
* @property {boolean} [intercept=false] - Indicates wether to automatically include the intercept.
*/
/**
 * Logistic regression model. Uses Newtons method to compute the weights.
 * <b>Before use: include BLAS library.</b>
 * @constructor
 * @param {(module:analytics~logisticRegParam|FIn)} [opts] - The options used for initialization or the input stream from which the model is loaded. If this parameter is an input stream than no other parameters are required.
 * @example
 * // import analytics module
 * var analytics = require('qminer').analytics;
 * // create the Logistic Regression model
 * var logreg = new analytics.LogReg({ lambda: 2 });
 */
 exports.LogReg = function (opts) { return Object.create(require('qminer').analytics.LogReg.prototype); }
/**
	* Gets the parameters.
	* @returns {module:analytics~logisticRegParam} The parameters of the model.
	* @example
	* // import analytics module
	* var analytics = require('qminer').analytics;
	* // create the Logistic Regression model
	* var logreg = new analytics.LogReg({ lambda: 10 });
	* // get the parameters of the model
	* var param = logreg.getParams(); // returns { lambda: 10, intercept: false }
	*/
 exports.LogReg.prototype.getParams = function () { return { lambda: 1.0, intercept: false } };
/**
	* Set the parameters.
	* @param {module:analytics~logisticRegParam} param - The new parameters.
	* @returns {module:analytics.LogReg} Self. The parameters are updated.
	*/
 exports.LogReg.prototype.setParams = function () { return Object.create(require('qminer').analytics.LogReg.prototype); }
/**
	 * Fits a column matrix of feature vectors X onto the response variable y.
	 * @param {module:la.Matrix} X - the column matrix which stores the feature vectors.
	 * @param {module:la.Vector} y - the response variable.
	 * @param {number} [eps] - the epsilon used for convergence.
	 * @returns {module:analytics.LogReg} Self.
	 */
 exports.LogReg.prototype.fit = function (X, y, eps) { return Object.create(require('qminer').analytics.LogReg.prototype); }
/**
	 * Returns the expected response for the provided feature vector.
	 * @param {module:la.Vector} x - the feature vector.
	 * @returns {number} the expected response.
	 */
 exports.LogReg.prototype.predict = function (x) { return 0.0; } 
/**
	 * Gives the weights of the model.
	 */
 exports.LogReg.prototype.weights = Object.create(require('qminer').analytics.LogReg.prototype);
/**
	 * Saves the model into the output stream.
	 * @param {module:fs.FOut} fout - the output stream.
	 * @returns {module:fs.FOut} The output stream fout. 
	 */
 exports.LogReg.prototype.save = function (fout) { return Object.create(require('qminer').fs.FOut.prototype); }
/**
* @typedef {Object} hazardModelParam
* The constructor parameters for the Proportional Hazards Model.
* @property {number} [lambda = 0] - The regularization parameter.
*/
/**
 * Proportional Hazards Model with a constant hazard function.
 * Uses Newtons method to compute the weights.
 * <b>Before use: include BLAS library.</b>
 *
 * @constructor
 * @property {module:analytics~hazardModelParam|FIn} [opts] - The options used for initialization or the input stream from which the model is loaded. If this parameter is an input stream than no other parameters are required.
 */
 exports.PropHazards = function (opts) { return Object.create(require('qminer').analytics.PropHazards.prototype); }
/**
	* Gets the parameters of the model.
	* @returns {module:analytics~hazardModelParam} The parameters of the model.
	*/
 exports.PropHazards.prototype.getParams = function () { return { lambda: 0.0 }; }
/**
	* Sets the parameters of the model.
	* @param {module:analytics~hazardModelParam} params - The parameters given to the model.
	* @returns {module:analytics.PropHazards} Self.
	*/
 exports.PropHazards.prototype.setParams = function (params) { return Object.create(require('qminer').analytics.PropHazards.prototype); }
/**
	 * Fits a column matrix of feature vectors X onto the response variable y.
	 *
	 * @param {module:la.Matrix} X - The column matrix which stores the feature vectors.
	 * @param {module:la.Vector} y - The response variable.
	 * @param {number} [eps] - The epsilon used for convergence.
	 * @returns {module:analytics.PropHazards} Self.
	 */
 exports.PropHazards.prototype.fit = function(X, y, eps) { return Object.create(require('qminer').analytics.PropHazards.prototype); }
/**
	 * Returns the expected response for the provided feature vector.
	 *
	 * @param {module:la.Vector} x - The feature vector.
	 * @returns {number} The expected response.
	 */
 exports.PropHazards.prototype.predict = function(x) { return 0.0; }
/**
	 * The models weights.
	 */
 exports.PropHazards.prototype.weights = Object.create(require('qminer').la.Vector.prototype);
/**
	 * Saves the model into the output stream.
	 * @param {module:fs.FOut} sout - The output stream.
	 * @returns {module:fs.FOut} The output stream sout.
	 */
 exports.PropHazards.prototype.save = function(sout) { return Object.create(require('qminer').fs.FOut.prototype); }
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
	 * Returns the centroid of the specified state containing only the observation parameters.
	 *
	 * @param {Number} stateId - the ID of the state
	 * @param {Boolean} [observations=true] - indicates wether to output observation or control coordinates
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
	 * Sets a callback function which is fired when a prediction is made. 4 paramters are passed
	 * to the callback:
	 * - Id of the target state
	 * - probability of occurring
	 * - vector of probabilities
	 * - vector of times corresponding to those probabilities
	 *
	 * @param {function} callback - the funciton which is called
	 */
/**
	 * Rebuilds its hierarchy.
	 */
/**
	 * Rebuilds the histograms using the instances stored in the columns of X.
	 *
	 * @param {Matrix} obsMat - the column matrix containing observation data instances
	 * @param {Matrix} controlMat - the column matrix containing control data instances
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
	 * Returns true if the state is a target on the specified height.
	 *
	 * @param {Number} stateId - Id of the state
	 * @param {Number} height - the height
	 * @returns {Boolean}
	 */
/**
	 * Sets whether the specified state is a target state or not.
	 *
	 * @param {Number} stateId - ID of the state
	 * @param {Number} height - the height on which the state is a target
	 * @param {Boolean} isTarget - set target on/off
	 */
/**
	 * Sets the factor of the specified control:
	 *
	 * @param {Number} ftrIdx - the index of the control feature
	 * @param {Number} factor
	 */
/**
	 * Saves the model to the output stream.
	 *
	 * @param {FOut} fout - the output stream
	 */

    exports.preprocessing = new function() {
        this.binarize = function (y, labelId) {
            var target = new la.Vector();
            for (var i = 0; i < y.length; i++) {
                target.push(y[i] === labelId ? 1 : -1);
            }
            return target;
        };

        this.applyModel = function (model, X) {
            var target = new la.Vector();
            for (var i = 0; i < X.cols; i++) {
                target.push(model.decisionFunction(X[i]));
            }
            return target;
        }
    };

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

    // var model = new OneVsAll({
    //     model : analytics.SVC,
    //     modelParam: { c: 10, j: 10, maxTime: 123 },
    //     cats : 123
    // });
    //
    // var X = featureSpace.extractSparseMatrix(recordSet);
    // var y = store.getCol("label");
    // model.fit(X, y);
    //
    // model.predict(featureSpace.extractSparseVector(record));

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
                var target = exports.preprocessing.binarize(y, cat);
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

    exports.metrics = new function() {
        // For evaluating provided categories (precision, recall, F1).
        this.ClassificationScore  = function (yTrue, yPred) {
            this.scores = {
                count: 0, predictionCount: 0,
                TP: 0, TN: 0, FP: 0, FN: 0,
                all: function () { return this.TP + this.FP + this.TN + this.FN; },
                precision: function () { return (this.FP == 0) ? 1 : this.TP / (this.TP + this.FP); },
                recall: function () { return this.TP / (this.TP + this.FN); },
                f1: function () { return 2 * this.precision() * this.recall() / (this.precision() + this.recall()); },
                accuracy: function () { return (this.TP + this.TN) / this.all(); }
            };

            // adds prediction to the current statistics. `correct` corresponds to the correct
            // label(s), `predicted` correspond to predicted lable(s). Labels can be either integers
            // or integer array (when there are zero or more then one lables).
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
        };

        this.accuracyScore = function (yTrue, yPred) {
            return new this.ClassificationScore (yTrue, yPred).scores.accuracy();
        };

        this.precisionScore = function (yTrue, yPred) {
            return new this.ClassificationScore (yTrue, yPred).scores.precision();
        };

        this.recallScore = function (yTrue, yPred) {
            return new this.ClassificationScore (yTrue, yPred).scores.recall();
        };

        this.f1Score = function (yTrue, yPred) {
            return new this.ClassificationScore (yTrue, yPred).scores.accuracy();
        };

        // used for computing ROC curve and other related measures such as AUC;
        this.PredictionCurve = function (yTrue, yPred) {
            // count of all examples
            this.length = 0;
            // count of all the positive and negative examples
    		this.allPositives = 0;
    		this.allNegatives = 0;
    		// store of predictions and ground truths
    		this.grounds = new la.Vector();
    		this.predictions = new la.Vector();

            // add new measurement with ground score (1 or -1) and predicted value
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

            // get ROC parametrization sampled on `sample' points
    		this.roc = function (sample) {
    			// default sample size is 10
    			sample = sample || 10;
    			// sort according to predictions
    			var perm = this.predictions.sortPerm(false);
    			// maintaining the results as we go along
    			var TP = 0, FP = 0, ROC = [[0, 0]];
    			// for figuring out when to dump a new ROC sample
    			var next = Math.floor(perm.perm.length / sample);
    			// go over the sorted results
    			for (var i = 0; i < perm.perm.length; i++) {
    				// get the ground
    				var ground = this.grounds[perm.perm[i]];
    				// update TP/FP counts according to the ground
    				if (ground > 0) { TP++ } else { FP++; }
    				// see if time to do next save
    				next = next - 1;
    				if (next <= 0) {
    					// add new datapoint to the curve
    					ROC.push([FP/this.allNegatives, TP/this.allPositives]);
    					// setup next timer
    					next = Math.floor(perm.perm.length / sample);
    				}
    			}
    			// add the last point
    			ROC.push([1,1]);
    			// return ROC
    			return ROC;
    		}

            // get AUC of the current curve
    		this.auc = function (sample) {
    			// default sample size is 10
    			sample = sample || 10;
    	        // get the curve
    	        var curve = this.curve(sample);
    	        // compute the area
    	        var result = 0;
    	        for (var i = 1; i < curve.length; i++) {
    	            // get edge points
    	            var left = curve[i-1];
    	            var right = curve[i];
    	            // first the rectangle bellow
    	            result = result + (right[0] - left[0]) * left[1];
    	            // an then the triangle above
    	            result = result + (right[0] - left[0]) * (right[1] - left[1]) / 2;
    	        }
    	        return result;
    	    }

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

            // get precision recall curve sampled on `sample' points
            this.precisionRecallCurve = function (sample) {
                return this.evalPrecisionRecall(new function (sample, length) {
                    // default sample size is 10
                    this.sample = sample || 10;
                    // curve
                    this.curve = [[0, 1]];
                    // for figuring out when to dump a new ROC sample
                    this.next = Math.floor(length / (this.sample));
                    this.counter = this.next;
                    console.log(length, this.sample, this.next);
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

            // get break-even point, the value where precision and recall intersect
            this.breakEvenPoint = function () {
                return this.evalPrecisionRecall(new function () {
                    this.minDiff = 1.0; this.bep = -1.0;
                    this.update = function (yTrue, yPred, precision, recall) {
                        var diff = Math.abs(precision - recall);
                        if (diff < minDiff) { minDiff = diff; bep = (precision + recall) / 2; }
                    }
                    this.finish = function () { return this.bep; }
                }());
            }

            // gets threshold for prediction score, which results in the highest F1
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

            // gets threshold for prediction score, nearest to specified recall
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

            // gets threshold for prediction score, nearest to specified recall
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

        this.rocCurve = function (yTrue, yPred, sample) {
            return new this.PredictionCurve(yTrue, yPred).roc(sample);
        };

        this.rocAucScore = function (yTrue, yPred, sample) {
            return new this.PredictionCurve(yTrue, yPred).roc(sample);
        };

        this.precisionRecallCurve = function (yTrue, yPred, sample) {
            return new this.PredictionCurve(yTrue, yPred).precisionRecallCurve(sample);
        };

        this.breakEventPointScore = function (yTrue, yPred) {
            return new this.PredictionCurve(yTrue, yPred).breakEvenPoint();
        };

        this.bestF1Threshold = function (yTrue, yPred) {
            return new this.PredictionCurve(yTrue, yPred).bestF1();
        };

        this.desiredRecallThreshold = function (yTrue, yPred, desiredRecall) {
            return new this.PredictionCurve(yTrue, yPred).desiredRecall(desiredRecall);
        };

        this.desiredPrecisionThreshold = function (yTrue, yPred, desiredPrecision) {
            return new this.PredictionCurve(yTrue, yPred).desiredPrecision(desiredPrecision);
        };
    };


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
        param = param == undefined ? {} : param;

        // Fit params
        var iter = param.iter == undefined ? 100 : param.iter;
        var k = param.k == undefined ? 2 : param.k;
        var verbose = param.verbose == undefined ? false : param.verbose;
        var fitIdx = param.fitIdx == undefined ? undefined : param.fitIdx;

        // Model
        var C = undefined;
        var idxv = undefined;
        var norC2 = undefined;

        /**
        * Permutes centroid with given mapping.
        @param {object} mapping - object that contains the mappping. E.g. mapping[4]=2 means "map cluster 4 into cluster 2"
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
        }
        /**
        * Returns the model
        * @returns {Object} The model object whose keys are: C (centroids), norC2 (centroid norms squared) and idxv (cluster ids of the training data)
        * Returns the model.
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
            return param;
        }

        /**
        * Computes the centroids
        * @param {(module:la.Matrix | module:la.SparseMatrix)} A - Matrix whose columns correspond to examples.
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
        this.fit = function (X) {
            // select random k columns of X, returns a dense C++ matrix
            var selectCols = function (X, k) {
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
                idxv = la.findMaxIdx(D);

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
                idxvOld = idxv.slice();
                C = getCentroids(X, idxv, C); //drag
            }
            if (verbose) {
                w.toc("end");
            }
            norC2 = la.square(C.colNorms());
        };

        /**
        * Returns an vector of cluster id assignments
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
        * Saves KMeans internal state into (binary) file
        * @param {string} fname - Name of the file to write into.
        */
        this.save = function(fname){
			if (!C) {
				throw new Error("KMeans.save() - model not created yet");
			}

			var params_vec = new la.Vector();
			params_vec.push(iter);
			params_vec.push(k);
			params_vec.push(verbose ? 1.0 : 0.0);

			var xfs = qm.fs;
			var fout = xfs.openWrite(fname);
			C.save(fout);
			norC2.save(fout);
			(new la.Vector(idxv)).save(fout);
			params_vec.save(fout);
			fout.close();
			fout = null;
		}
		/**
        * Loads KMeans internal state from (binary) file
        * @param {string} fname - Name of the file to read from.
        */
        this.load = function (fname) {
		    var xfs = qm.fs;
		    var fin = xfs.openRead(fname);

		    C = new la.Matrix();
		    C.load(fin);
		    norC2 = new la.Vector();
		    norC2.load(fin);

		    var idxvtmp = new la.Vector();
		    idxvtmp.load(fin);
		    idxv = idxvtmp; // make normal vector (?)

		    var params_vec = new la.Vector();
		    params_vec.load(fin);
		    iter = params_vec[0];
		    k = params_vec[1];
		    verbose = (params_vec[2] != 0);

		    fin = null;
		}
    }

    ///////////////////////////////
    ////// code below not yet ported or verified for scikit
    ///////////////////////////////

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

	//!- `result = analytics.crossValidation(rs, features, target, folds)` -- creates a batch
    //!     model for records from record set `rs` using `features; `target` is the
    //!     target field and is assumed discrete; the result is a results object
    //!     with the following API:
    //!     - `result.target` -- an object with categories as keys and the following
    //!       counts as members of these keys: `count`, `TP`, `TN`, `FP`, `FN`,
    //!       `all()`, `precision()`, `recall()`, `accuracy()`.
    //!     - `result.confusion` -- confusion matrix between categories
    //!     - `result.report()` -- prints basic report on to the console
    //!     - `result.reportCSV(fout)` -- prints CSV output to the `fout` output stream
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
            if (!cfyRes) { cfyRes = new exports.ClassificationScore (model.target); }
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
     * StreamStory.
     * @class
     * @param {opts} HierarchMarkovParam - parameters. TODO typedef and describe
     */
    exports.HierarchMarkov = function (opts) {
    	// constructor
    	if (opts == null) throw 'Missing parameters!';
    	if (opts.base == null) throw 'Missing parameter base!';

    	// create model and feature space
    	var mc;
    	var obsFtrSpace;
    	var controlFtrSpace;

    	if (opts.hmcConfig != null && opts.obsFields != null &&
    			opts.contrFields != null && opts.base != null) {

    		mc = opts.sequenceEndV != null ? new exports.HMC(opts.hmcConfig, opts.sequenceEndV) : new exports.HMC(opts.hmcConfig);

    		obsFtrSpace = new qm.FeatureSpace(opts.base, opts.obsFields);
    		controlFtrSpace = new qm.FeatureSpace(opts.base, opts.contrFields);
    	}
    	else if (opts.hmcFile != null) {
    		var fin = new fs.FIn(opts.hmcFile);
    		mc = new exports.HMC(fin);
    		obsFtrSpace = new qm.FeatureSpace(opts.base, fin);
    		controlFtrSpace = new qm.FeatureSpace(opts.base, fin);
    	}
    	else {
    		throw 'Parameters missing: ' + JSON.stringify(opts);
    	}

    	function getFtrNames(ftrSpace) {
    		var names = [];

    		var dims = ftrSpace.dims;
    		for (var i = 0; i < dims.length; i++) {
				names.push(ftrSpace.getFeature(i));
			}

    		return names;
    	}

    	function getObsFtrCount() {
			return obsFtrSpace.dims.length;
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

			var coords = mc.fullCoords(stateId);
			var obsFtrNames = getObsFtrNames();
			var invObsCoords = obsFtrSpace.invertFeatureVector(coords);
			for (var i = 0; i < invObsCoords.length; i++) {
				observations.push({name: obsFtrNames[i], value: invObsCoords.at(i)});
			}

			var controlCoords = mc.fullCoords(stateId, false);
			var contrFtrNames = getControlFtrNames();
			var invControlCoords = controlFtrSpace.invertFeatureVector(controlCoords);
			for (var i = 0; i < invControlCoords.length; i++) {
				controls.push({name: contrFtrNames[i], value: invControlCoords.at(i)});
			}

			return {
				observations: observations,
				controls: controls
			};
    	}

    	function getFtrCoord(stateId, ftrIdx) {
    		if (ftrIdx < obsFtrSpace.dims.length) {
    			return obsFtrSpace.invertFeatureVector(mc.fullCoords(stateId))[ftrIdx];
    		} else {
    			return controlFtrSpace.invertFeatureVector(mc.fullCoords(stateId, false))[ftrIdx - obsFtrSpace.dims.length];
    		}
    	}

    	// public methods
    	var that = {
    		/**
    		 * Creates a new model out of the record set.
    		 */
    		fit: function (opts) {
    			var recSet = opts.recSet;
    			var batchEndV = opts.batchEndV;
    			var timeField = opts.timeField;

    			log.info('Updating feature space ...');
    			obsFtrSpace.updateRecords(recSet);
    			controlFtrSpace.updateRecords(recSet);

    			var obsColMat = obsFtrSpace.extractMatrix(recSet);
    			var contrColMat = controlFtrSpace.extractMatrix(recSet);
    			var timeV = recSet.getVector(timeField);

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

    		getFtrNames: function () {
    			return {
    				observation: getObsFtrNames(),
    				control: getControlFtrNames()
    			}
    		},

    		/**
    		 * Returns state details as a Javascript object.
    		 */
    		stateDetails: function (stateId, height) {
    			var futureStates = mc.futureStates(height, stateId);
    			var pastStates = mc.pastStates(height, stateId);
    			var isTarget = mc.isTarget(stateId, height);
    			var stateNm = mc.getStateName(stateId);
    			var wgts = mc.getStateWgtV(stateId);

    			var features = getFtrDescriptions(stateId);

    			return {
    				id: stateId,
    				name: stateNm.length > 0 ? stateNm : null,
    				isTarget: isTarget,
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

    			var nObsFtrs = getObsFtrCount();

    			if (ftrIdx < nObsFtrs) {
	    			for (var i = 0; i < hist.binStartV.length; i++) {
	    				hist.binStartV[i] = obsFtrSpace.invertFeature(ftrIdx, hist.binStartV[i]);
	    			}
    			} else {
    				for (var i = 0; i < hist.binStartV.length; i++) {
	    				hist.binStartV[i] = controlFtrSpace.invertFeature(ftrIdx - nObsFtrs, hist.binStartV[i]);
	    			}
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

    		setControl: function (ftrIdx, factor) {
    			var controlFtrIdx = ftrIdx - obsFtrSpace.dims.length;
    			mc.setControlFactor(controlFtrIdx, factor);
    		}
    	};

    	return that;
    };
    

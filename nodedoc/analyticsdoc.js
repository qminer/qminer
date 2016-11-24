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
 * // import modules
 * var qm = require('qminer');
 * var analytics = qm.analytics;
 * // load dataset, create model, evaluate model
 */
/**
    * Calculates the non-negative matrix factorization, see: {@link https://en.wikipedia.org/wiki/Non-negative_matrix_factorization}.
    * @param {(module:la.Matrix | module:la.SparseMatrix)} mat - The non-negative matrix.
    * @param {number} k - The reduced rank, e.g. number of columns in matrix U and number of rows in matrix V. Must be between 0 and `min(mat.rows, mat.cols)`.
    * @param {Object} [json] - Algorithm options.
    * @param {number} [json.iter = 100] - The number of iterations used for the algorithm.
    * @param {number} [json.tol = 1e-3] - The tolerance.
    * @param {boolean} [json.verbose = false] - If false, the console output is supressed.
    * @returns {Object} The json object `nmfRes` containing the non-negative matrices U and V:
    * <br> `nmfRes.U`- The {@link module:la.Matrix} representation of the matrix U,
    * <br> `nmfRes.V`- The {@link module:la.Matrix} representation of the matrix V.
    * @example <caption>Asynchronous function</caption>
    * // import modules
    * var analytics = require('qminer').analytics;
    * var la = require('qminer').la;
    * // create a matrix
    * var mat = new la.Matrix({ rows: 10, cols: 5, random: true });
    * // compute the non-negative matrix factorization
    * analytics.nmfAsync(mat, 3, { iter: 100, tol: 1e-4 }, function (err, result) {
    *    if (err) { console.log(err); }
    *    // calculation successful
    *    var U = result.U;
    *    var V = result.V;
    * });
    * @example <caption>Synchronous function</caption>
    * // import modules
    * var analytics = require('qminer').analytics;
    * var la = require('qminer').la;
    * // create a matrix
    * var mat = new la.Matrix({ rows: 10, cols: 5, random: true });
    * // compute the non-negative matrix factorization
    * var result = analytics.nmf(mat, 3, { iter: 100, tol: 1e-4 });
    * var U = result.U;
    * var V = result.V;
    */
 exports.prototype.nmf = function (mat, k, json) { return { "U": Object.create(require('qminer').la.Matrix.prototype), "V": Object.create(require('qminer').la.Matrix.prototype) }; }
/**
* @typedef {Object} SVMParam
* SVM constructor parameters. Used for the construction of {@link module:analytics.SVC} and {@link module:analytics.SVR}.
* @property  {string} [algorithm='SGD'] - The algorithm procedure. Possible options are `'SGD'`, `'PR_LOQO'` and `'LIBSVM'`.
* @property  {number} [c=1.0] - Cost parameter. Increasing the parameter forces the model to fit the training data more accurately (setting it too large may lead to overfitting) .
* @property  {number} [j=1.0] - Unbalance parameter. Increasing it gives more weight to the positive examples (getting a better fit on the positive training examples gets a higher priority). Setting c=n is like adding n-1 copies of the positive training examples to the data set.
* @property  {number} [eps=1e-1] - Epsilon insensitive loss parameter. Larger values result in fewer support vectors (smaller model complexity)
* @property  {number} [batchSize=1000] - Number of examples used in the subgradient estimation. Higher number of samples slows down the algorithm, but makes the local steps more accurate.
* @property  {number} [maxIterations=10000] - Maximum number of iterations.
* @property  {number} [maxTime=1] - Maximum runtime in seconds.
* @property  {number} [minDiff=1e-6] - Stopping criterion tolerance.
* @property  {boolean} [verbose=false] - Toggle verbose output in the console.
*/
/**
* SVC
* @classdesc Support Vector Machine Classifier. Implements a soft margin linear support vector classifier using the PEGASOS algorithm, 
* see: {@link http://ttic.uchicago.edu/~nati/Publications/PegasosMPB.pdf Pegasos: Primal Estimated sub-GrAdient SOlver for SVM}.
* @class
* @param {module:analytics~SVMParam | module:fs.FIn} [arg] - Construction arguments. There are two ways of constructing:
* <br>1. Using the {@link  module:analytics~SVMParam} object,
* <br>2. using the file input stream {@link module:fs.FIn}.
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
* var SVC = new analytics.SVC({ verbose: false });
* // train classifier
* SVC.fit(featureMatrix, targets);
* // set up a fake test vector
* var test = new la.Vector([1.1, -0.5]);
* // predict the target value
* var prediction = SVC.predict(test);
*/
 exports.SVC = function(arg) { return Object.create(require('qminer').analytics.SVC.prototype); };
/**
    * Gets the SVC parameters.
    * @returns {module:analytics~SVMParam} Parameters of the classifier model.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create a new SVC model with json
    * var SVC = new analytics.SVC({ c: 5, j: 10, batchSize: 2000, maxIterations: 12000, maxTime: 2, minDiff: 1e-10, verbose: true });
    * // get the parameters of the SVC model
    * // returns { algorithm: 'SGD' c: 5, j: 10, eps: 0.1, batchSize: 2000, maxIterations: 12000, maxTime: 2, minDiff: 1e-10, verbose: true }
    * var json = SVC.getParams(); 
    */
 exports.SVC.prototype.getParams = function() { return { algorithm: '', c: 0, j: 0, eps: 0.1, batchSize: 0, maxIterations: 0, maxTime: 0, minDiff: 0, verbose: true } };
/**
    * Sets the SVC parameters.
    * @param {module:analytics~SVMParam} param - Classifier training parameters.
    * @returns {module:analytics.SVC} Self. Updated the training parameters.
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
    * Gets the vector of coefficients of the linear model. Type {@link module:la.Vector}.
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
    * @returns {module:fs.FOut} The output stream `fout`.
    * @example
    * // import the analytics and la modules
    * var analytics = require('qminer').analytics;
    * var la = require('qminer').la;
    * var fs = require('qminer').fs;
    * // create a new SVC object
    * var SVC = new analytics.SVC();
    * // create the matrix containing the input features and the input vector for each matrix column.
    * var matrix = new la.Matrix([[1, 0, -1, 0], [0, 1, 0, -1]]);    
    * var vec = new la.Vector([1, 0, -1, -2]);
    * // fit the model
    * SVC.fit(matrix, vec);
    * // create output stream
    * var fout = fs.openWrite('svc_example.bin');
    * // save SVC object (model and parameters) to output stream and close it
    * SVC.save(fout);
    * fout.close();
    * // create input stream
    * var fin = fs.openRead('svc_example.bin');
    * // create a SVC object that loads the model and parameters from input stream
    * var SVC2 = new analytics.SVC(fin);    
    */
 exports.SVC.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); }
/**
    * Sends vector through the model and returns the distance to the decision boundery.
    * @param {module:la.Vector | module:la.SparseVector | module:la.Matrix | module:la.SparseMatrix} X - Input feature vector or matrix with feature vectors as columns.
    * @returns {number | module:la.Vector} Distance:
    * <br>1. Real number, if `X` is {@link module:la.Vector} or {@link module:la.SparseVector}.
    * <br>2. {@link module:la.Vector}, if `X` is {@link module:la.Matrix} or {@link module:la.SparseMatrix}.
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
    * <br>1. Real number, if `X` is {@link module:la.Vector} or {@link module:la.SparseVector}.
    * <br>2. {@link module:la.Vector}, if `X` is {@link module:la.Matrix} or {@link module:la.SparseMatrix}.
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
    * @returns {module:analytics.SVC} Self. The model has been created.
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
* SVR
* @classdesc Support Vector Machine Regression. Implements a soft margin linear support vector regression using the PEGASOS algorithm with epsilon insensitive loss, see: {@link http://ttic.uchicago.edu/~nati/Publications/PegasosMPB.pdf Pegasos: Primal Estimated sub-GrAdient SOlver for SVM}.
* @class
* @param {module:analytics~SVMParam | module:fs.FIn} [arg] - Construction arguments. There are two ways of constructing:
* <br>1. Using the {@link  module:analytics~SVMParam} object,
* <br>2. using the file input stream {@link module:fs.FIn}.
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
* var SVR = new analytics.SVR({ verbose: false });
* // Train regression
* SVR.fit(featureMatrix, targets);
* // Set up a fake test vector
* var test = new la.Vector([1.1, -0.8]);
* // Predict the target value
* var prediction = SVR.predict(test);
*/
 exports.SVR = function(arg) { return Object.create(require('qminer').analytics.SVR.prototype); };
/**
    * Gets the SVR parameters.
    * @returns {module:analytics~SVMParam} Parameters of the regression model.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create a new SVR object
    * var SVR = new analytics.SVR({ c: 10, eps: 1e-10, maxTime: 12000, verbose: true });
    * // get the parameters of SVR
    * var params = SVR.getParams();
    */
 exports.SVR.prototype.getParams = function() { return { algorithm: '', c: 0, j: 0, eps: 0, batchSize: 0, maxIterations: 0, maxTime: 0, minDiff: 0, verbose: true } };
/**
    * Sets the SVR parameters.
    * @param {module:analytics~SVMParam} param - Regression training parameters.
    * @returns {module:analytics.SVR} Self. Updated the training parameters.
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
    * The vector of coefficients of the linear model. Type {@link module:la.Vector}.
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
    * // get the coeficients of the linear model
    * var coef = SVR.weights;
    */
 exports.SVR.prototype.weights = Object.create(require('qminer').la.Vector.prototype);
/**
    * Saves model to output file stream.
    * @param {module:fs.FOut} fout - Output stream.
    * @returns {module:fs.FOut} The output stream `fout`.
    * @example
    * // import the modules
    * var analytics = require('qminer').analytics;
    * var la = require('qminer').la;
    * var fs = require('qminer').fs;
    * // create a new SVR object
    * var SVR = new analytics.SVR({ c: 10 });
    * // create a matrix and vector for the model
    * var matrix = new la.Matrix([[1, -1], [1, 1]]);
    * var vector = new la.Vector([1, 1]);
    * // create the model by fitting the values
    * SVR.fit(matrix, vector);
    * // save the model in a binary file
    * var fout = fs.openWrite('svr_example.bin');
    * SVR.save(fout);
    * fout.close();
    * // construct a SVR model by loading from the binary file
    * var fin = fs.openRead('svr_example.bin');
    * var SVR2 = new analytics.SVR(fin);
    */
 exports.SVR.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); }
/**
     * Sends vector through the model and returns the scalar product as a real number.
     * @param {module:la.Vector | module:la.SparseVector | module:la.Matrix | module:la.SparseMatrix} X - Input feature vector or matrix with feature vectors as columns.
     * @returns {number | module:la.Vector} Distance:
     * <br>1. Real number if `X` is {@link module:la.Vector} or {@link module:la.SparseVector}.
     * <br>2. {@link module:la.Vector}, if `X` is {@link module:la.Matrix} or {@link module:la.SparseMatrix}.
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
    * <br>1. Real number, if `X` is {@link module:la.Vector} or {@link module:la.SparseVector}.
    * <br>2. {@link module:la.Vector}, if `X` is {@link module:la.Matrix} or {@link module:la.SparseMatrix}.
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
    * Fits a SVM regression model, given column examples in a matrix and vector of targets.
    * @param {module:la.Matrix | module:la.SparseMatrix} X - Input feature matrix where columns correspond to feature vectors.
    * @param {module:la.Vector} y - Input vector of targets, one for each column of X.
    * @returns {module:analytics.SVR} Self.  The model has been created.
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
* An object used for the construction of {@link module:analytics.RidgeReg}.
* @property {number} [gamma=0.0] - The gamma value.
*/
/**
 * Ridge Regression
 * @class
 * @classdesc Ridge regression minimizes the value `||A' x - b||^2 + ||gamma x||^2`. 
 * Uses {@link http://en.wikipedia.org/wiki/Tikhonov_regularization Tikhonov regularization}.
 * @param {module:analytics~ridgeRegParam | module:fs.FIn} [arg] - Construction arguments. There are two ways of constructing:
 * <br>1. Using the {@link  module:analytics~ridgeRegParam} object,
 * <br>2. using the file input stream {@link module:fs.FIn}.
 * @example
 * // import modules
 * analytics = require('qminer').analytics;
 * la = require('qminer').la;
 * // create a new model with gamma equal to 1.0
 * var regmod = new analytics.RidgeReg({ gamma: 1.0 });
 * // generate a random feature matrix
 * var A = la.randn(10, 100);
 * // generate a random model
 * var w = la.randn(10);
 * // generate noise
 * var n = la.randn(100).multiply(0.01);
 * // generate responses (model'*data + noise)
 * var b = A.transpose().multiply(w).plus(n);
 * // fit model
 * regmod.fit(A, b);
 * // compare the true with the trained model
 * // true model
 * w.print();
 * // trained model;
 * regmod.weights.print();
 * // cosine between the true and the estimated model should be close to 1 if the fit succeeded
 * var cos = regmod.weights.cosine(w);
 */
 exports.RidgeReg = function(arg) { return Object.create(require('qminer').analytics.RidgeReg.prototype) };
/**
    * Gets the parameters.
    * @returns {model:analytics~RidgeRegParam} The object containing the parameters.
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
    * @param {number | model:analytics~RidgeRegParam} gamma - The new parameter for the model, given as a number or as an object.
    * @returns {module:analytics.RidgeReg} Self. The parameter is set to `gamma`.
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
     * Fits a column matrix of feature vectors `X` onto the response variable `y`.
     * @param {module:la.Matrix} X - Column matrix which stores the feature vectors.
     * @param {module:la.Vector} y - Response variable.
     * @returns {module:analytics.RidgeReg} Self. The model is fitted by `X` and `y`.
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
     * Vector of coefficients for linear regression. Type {@link module:la.Vector}.
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
     * // get the weights
     * var weights = regmod.weights;
     */
 exports.RidgeReg.prototype.weights = Object.create(require('qminer').la.Vector.prototype);
/**
     * Saves the model into the output stream.
     * @param {module:fs.FOut} fout - Output stream.
     * @returns {module:fs.FOut} The output stream `fout`.
     * @example
     * // import modules
     * var analytics = require('qminer').analytics;
     * var la = require('qminer').la;
     * var fs = require('qminer').fs;
     * // create a new Ridge Regression object
     * var regmod = new analytics.RidgeReg();
     * // create the test matrix and vector
     * var X = new la.Matrix([[1, 2], [1, -1]]);
     * var y = new la.Vector([3, 3]);
     * // fit the model with X and y
     * regmod.fit(X, y);
     * // create an output stream object and save the model
     * var fout = fs.openWrite('regmod_example.bin');
     * regmod.save(fout);
     * fout.close();
     * // create a new Ridge Regression model by loading the model
     * var fin = fs.openRead('regmod_example.bin');
     * var regmod2 = new analytics.RidgeReg(fin);
     */
 exports.RidgeReg.prototype.save = function(fout) { Object.create(require('qminer').fs.FOut.prototype); };
/**
 * Sigmoid function (`y = 1/[1 + exp[-A*x + B]]`) fitted on decision function to mimic.
 * @class
 * @param {module:fs.FIn} [arg] - Construction arguments.
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
 exports.Sigmoid = function(arg) { return Object.create(require('qminer').analytics.Sigmoid.prototype); };
/**
    * Get the parameters. <i>It doesn't do anything, it's only for consistency for constructing pipeline.</i>
    * @returns {Object} An empty object.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create the Sigmoid model
    * var s = new analytics.Sigmoid();
    * // get the parameters
    * // returns an empty object
    * var param = s.getParams();
    */
 exports.Sigmoid.prototype.getParams = function () { return {}; }
/**
    * Sets the parameters. <i>It doesn't do anything, it's only for consistency for constructing pipeline.</i>
    * @param {Object} arg - Json object. 
    * @returns {module:analytics.Sigmoid} Self. Nothing changes.
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
    * @returns {Object} The object `sigModel` containing the properties:
    * <br> `sigModel.A` - First value of the Sigmoid model,
    * <br> `sigModel.B` - Second value of the Sigmoid model.
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
     * Fits a column matrix of feature vectors `X` onto the response variable `y`.
     * @param {module:la.Vector} x - Predicted values (e.g. using {@link module:analytics.SVR}).
     * @param {module:la.Vector} y - Actual binary labels: 1 or -1.
     * @returns {module:analytics.Sigmoid} Self. The model has been created.
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
     * s.fit(X, y);
     */
 exports.Sigmoid.prototype.fit = function(X, y) { return Object.create(require('qminer').analytics.Sigmoid.prototype); }
/**
     * Returns the expected response for the provided feature vector.
     * @param {number | module:la.Vector} x - Prediction score.
     * @returns {number | module:la.Vector}
     * <br> 1. If `x` is a number, returns a normalized prediction score,
     * <br> 2. if `x` is a {@link module:la.Vector}, returns a vector of normalized prediction scores.
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
 exports.Sigmoid.prototype.decisionFunction = function(x) { return (x instanceof Object.create(require('qminer').la.Vector)) ? Object.create(require('qminer').la.Vector.prototype) : 0.0; }
/**
     * Returns the expected response for the provided feature vector.
     * @param {number | module:la.Vector} x - Prediction score.
     * @returns {number | module:la.Vector} 
     * <br> 1. If `x` is a number, returns a normalized prediction score,
     * <br> 2. if `x` is a {@link module:la.Vector}, returns a vector of normalized prediction scores.
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
 exports.Sigmoid.prototype.predict = function(x) { return (x instanceof Object.create(require('qminer').la.Vector)) ? Object.create(require('qminer').la.Vector.prototype) : 0.0; }
/**
     * Saves the model into the output stream.
     * @param {module:fs.FOut} fout - Output stream.
     * @returns {module:fs.FOut} The output stream `fout`.
     * @example
     * // import modules
     * var analytics = require('qminer').analytics;
     * var la = require('qminer').la;
     * var fs = require('qminer').fs;
     * // create the Sigmoid model
     * var s = new analytics.Sigmoid();
     * // create the predicted values and the binary labels
     * var X = new la.Vector([-3, -2, -1, 1, 2, 3]);
     * var y = new la.Vector([-1, -1, -1, 1, 1, 1]);
     * // fit the model
     * s.fit(X, y);
     * // create an output stream object and save the model
     * var fout = fs.openWrite('sigmoid_example.bin');
     * s.save(fout);
     * fout.close();
     * // create a new Sigmoid model by loading the model
     * var fin = fs.openRead('sigmoid_example.bin');
     * var s2 = new analytics.Sigmoid(fin);
     */
 exports.Sigmoid.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); };
/**
* @typedef {Object} detectorParam
* An object used for the construction of {@link module:analytics.NearestNeighborAD}.
* @param {number} [rate=0.05] - The expected fracton of emmited anomalies (0.05 -> 5% of cases will be classified as anomalies).
* @param {number} [windowSize=100] - Number of most recent instances kept in the model.
*/
/**
 * Nearest Neighbour Anomaly Detection 
 * @classdesc Anomaly detector that checks if the test point is too far from the nearest known point.
 * @class
 * @param {module:analytics~detectorParam | module:fs.FIn} [arg] - Construction arguments. There are two ways of constructing:
* <br>1. Using the {@link  module:analytics~detectorParam} object,
* <br>2. using the file input stream {@link module:fs.FIn}.
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
    * @param {module:analytics~detectorParam} params - The object containing the parameters.
    * @returns {module:analytics.NearestNeighborAD} Self. The parameters are updated with `params`.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create a new NearestNeighborAD object
    * var neighbor = new analytics.NearestNeighborAD();
    * // set it's parameters to rate: 0.1
    * neighbor.setParams({ rate: 0.1 });
    */
 exports.NearestNeighborAD.prototype.setParams = function (params) { return Object.create(require('qminer').analytics.NearestNeighborAD.prototype); }
/**
    * Gets parameters.
    * @returns {module:analytics~detectorParam} The object containing the parameters.
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
     * Saves model to provided output stream.
     * @param {module:fs.FOut} fout - The output stream.
     * @returns {module:fs.FOut} The output stream `fout`.
     * @example
     * // import modules
     * var analytics = require('qminer').analytics;
     * var la = require('qminer').la;
     * var fs = require('qminer').fs;
     * // create a new NearestNeighborAD object
     * var neighbor = new analytics.NearestNeighborAD();
     * // create a new sparse matrix
     * var matrix = new la.SparseMatrix([[[0, 1], [1, 2]], [[0, -2], [1, 3]], [[0, 0], [1, 1]]]);
     * // fit the model with the matrix
     * neighbor.fit(matrix);
     * // create an output stream object and save the model
     * var fout = fs.openWrite('neighbor_example.bin');
     * neighbor.save(fout);
     * fout.close();
     * // create a new Nearest Neighbor Anomaly model by loading the model
     * var fin = fs.openRead('neighbor_example.bin');
     * var neighbor2 = new analytics.NearestNeighborAD(fin);
     */
 exports.NearestNeighborAD.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); }
/**
    * Returns the model.
    * @returns {Object} The object `neighbourModel` containing the properties:
    * <br> 1. `neighbourModel.rate` - The expected fraction of emmited anomalies.
    * <br> 2. `neighbourModel.thresh` - Maximal squared distance to the nearest neighbor that is not anomalous.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create a new NearestNeighborAD object
    * var neighbor = new analytics.NearestNeighborAD({ rate: 0.1 });
    * // get the model of the object
    * // returns a json object { rate: 0.1, window: 0 }
    * var model = neighbor.getModel();
    */
 exports.NearestNeighborAD.prototype.getModel = function () { return { rate: 0.1, threshold: 0.0 }; }
/**
    * Adds a new point to the known points and recalculates the threshold.
    * @param {module:la.SparseVector} X - Test example.
    * @param {number} recId - Integer record ID, used in {@link module:analytics.NearestNeighborAD.prototype.explain}.
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
    * Analyzes the nearest neighbor distances and calculates the detector threshold based on the rate parameter.
    * @param {module:la.SparseMatrix} A - Matrix whose columns correspond to known examples. Gets saved as it is part of the model.
    * @param {module:la.IntVector} [idVec] - An integer vector of IDs.
    * @returns {module:analytics.NearestNeighborAD} Self. The model is set by the matrix `A`.
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
 exports.NearestNeighborAD.prototype.fit = function(A, idVec) { return Object.create(require('qminer').NearestNeighborAD.prototype); }
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
    * Compares the point to the known points and returns 1 if it's too far away (based on the precalculated threshold).
    * @param {module:la.SparseVector} x - Test vector.
    * @returns {number} Returns 1.0 if the vector `x` is an anomaly and 0.0 otherwise.
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
    * @typedef {Object} NearestNeighborADExplain
    * An object used for interpreting the predictions of {@link module:analytics.NearestNeighborAD#explain}.
    * @property {number} nearestID - The ID of the nearest neighbor.
    * @property {number} distance - The distance to the nearest neighbor.
    * @property {Array.<module:analytics~NearestNeighborADFeatureContribution>} features - An array with feature contributions.
    * @property {number} oldestID - The ID of the oldest record in the internal buffer (the record that was added first).
    * @property {number} newestID - The ID of the newest record in the internal buffer (the record that was added last).
    */
/**
    * @typedef {Object} NearestNeighborADFeatureContribution
    * An object explaining the prediction of {@link module:analytics.NearestNeighborAD#explain} in terms of a single feature.
    * Contained in the object {@link module:analytics~NearestNeighborADExplain}.
    * @property {number} id - The ID of the feature.
    * @property {number} val - The value of the feature for the vector we are explaining.
    * @property {number} nearVal - The the value of the feature for the nearest neighbor.
    * @property {number} contribution - Fraction of the total distance `(v(i) - n(i))^2 / ||v - n||^2`.
    */
/**
    * Returns an object that encodes the ID of the nearest neighbor and the features that contributed to the distance.
    * @param {module:la.SparseVector} x - Test vector.
    * @returns {module:analytics~NearestNeighborADExplain} The explanation object.
    * @example
    * // import modules
    * var analytics = require('qminer').analytics;
    * var la = require('qminer').la;
    * // create a new NearestNeighborAD object
    * var neighbor = new analytics.NearestNeighborAD({ rate:0.05, windowSize:3 });
    * // create a new sparse matrix
    * var matrix = new la.SparseMatrix([[[0, 1], [1, 2]], [[0, -2], [1, 3]], [[0, 0], [1, 1]]]);
    * // fit the model with the matrix and provide a vector record IDs
    * neighbor.fit(matrix, new la.IntVector([3541, 1112, 4244]));
    * // create a new sparse vector
    * var vector = new la.SparseVector([[0, 4], [1, 0]]);
    * // check if the vector is an anomaly
    * var explanation = neighbor.explain(vector); // returns an explanation
    */
 exports.NearestNeighborAD.prototype.explain = function(x) { return {}; }
/**
    * Returns true when the model has enough data to initialize. Type `boolean`.
    * @example
    * // import modules
    * var analytics = require('qminer').analytics;
    * var la = require('qminer').la;
    * // create a new NearestNeighborAD object
    * var neighbor = new analytics.NearestNeighborAD({ rate:0.05, windowSize:3 });
    * // check if the model has enough data
    * neighbor.init;
    */
 exports.NearestNeighborAD.prototype.init = false;
/**
* @typedef {Object} recLinRegParam
* An object used for the construction of {@link module:analytics.RecLinReg}.
* @param {number} dim - The dimension of the model.
* @param {number} [regFact=1.0] - The regularization factor.
* @param {number} [forgetFact=1.0] - The forgetting factor.
*/
/**
* Recursive Linear Regression
* @classdesc Holds the Recursive Linear Regression model.
* @class
* @param {module:analytics~recLinRegParam | module:fs.FIn} arg - Construction arguments. There are two ways of constructing:
* <br>1. Using the {@link  module:analytics~detectorParam} object,
* <br>2. using the file input stream {@link module:fs.FIn}.
* @example
* // import analytics module
* var analytics = require('qminer').analytics;
* // create the recursive linear regression model holder
* var linreg = new analytics.RecLinReg({ dim: 10, regFact: 1.0, forgetFact: 1.0 });
*/
 exports.RecLinReg = function(arg) { return Object.create(require('qminer').analytics.RecLinReg.prototype); }
/**
    * Updates the internal model.
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
    * Creates/updates the internal model.
    * @param {module:la.Matrix} mat - The input matrix.
    * @param {module:la.Vector} vec - The target numbers, where the i-th number in vector is the target number for the i-th column of the `mat`.
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
    * @param {module:la.Vector} vec - The prediction vector.
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
    * @param {module:analytics~recLinRegParam} params - The new parameters of the model.
    * @returns {module:analytics.RecLinReg} Self. The parameters are updated. Any previous model is set to default.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create a new Recursive Linear Regression model
    * var linreg = new analytics.RecLinReg({ dim: 10 });
    * // set the parameters of the model
    * linreg.setParams({ dim: 3, recFact: 1e2, forgetFact: 0.5 });
    */
 exports.RecLinReg.prototype.setParams = function (params) { return Object.create(require('qminer').analytics.RecLinReg.prototype); }
/**
    * Returns the parameters.
    * @returns {module:analytics~recLinRegParam} The parameters of the model.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create a new Recursive Linear Regression model
    * var linreg = new analytics.RecLinReg({ dim: 10 });
    * // get the parameters of the model
    * var params = linreg.getParams(); // returns { dim: 10, recFact: 1.0, forgetFact: 1.0 }
    */
 exports.RecLinReg.prototype.getParams = function () { return { dim: 0, regFact: 1.0, forgetFact: 1.0 }; }
/**
    * Gives the weights of the model. Type {@link module:la.Vector}.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * var la = require('qminer').la;
    * // create a new Recursive Linear Regression model
    * var linreg = new analytics.RecLinReg({ dim: 2 });
    * // create a new dense matrix and target vector
    * var mat = new la.Matrix([[1, 2], [1, -1]]);
    * var vec = new la.Vector([3, 3]);
    * // fit the model with the matrix
    * linreg.fit(mat, vec);
    * // get the weights of the model
    * var weights = linreg.weights;
    */
 exports.RecLinReg.prototype.weights = Object.create(require('qminer').la.Vector.prototype);
/**
    * Gets the dimensionality of the model. Type `number`.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create a new Recursive Linear Regression model
    * var linreg = new analytics.RecLinReg({ dim: 10 });
    * // get the dimensionality of the model
    * var dim = linreg.dim;
    */
 exports.RecLinReg.prototype.dim = 0;
/**
    * Save model to provided output stream.
    * @param {module:fs.FOut} fout - The output stream.
    * @returns {module:fs.FOut} The output stream `fout`.
    * @example
    * // import modules
    * var analytics = require('qminer').analytics;
    * var la = require('qminer').la;
    * var fs = require('qminer').fs;
    * // create the Recursive Linear Regression model
    * var linreg = new analytics.RecLinReg({ dim: 2.0, recFact: 1e-10 });
    * // create a new dense matrix and target vector
    * var mat = new la.Matrix([[1, 2], [1, -1]]);
    * var vec = new la.Vector([3, 3]);
    * // fit the model with the matrix
    * linreg.fit(mat, vec);
    * // create an output stream object and save the model
    * var fout = fs.openWrite('linreg_example.bin');
    * linreg.save(fout);
    * fout.close();
    * // create a new Nearest Neighbor Anomaly model by loading the model
    * var fin = fs.openRead('linreg_example.bin');
    * var linreg2 = new analytics.RecLinReg(fin);
    */
 exports.RecLinReg.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); }
/**
* @typedef {Object} logisticRegParam
* An object used for the construction of {@link module:analytics.LogReg}.
* @property {number} [lambda=1] - The regularization parameter.
* @property {boolean} [intercept=false] - Indicates whether to automatically include the intercept.
*/
/**
 * Logistic regression model.
 * @class
 * @classdesc  Uses Newtons method to compute the weights. <b>Before use: QMiner must be built with the OpenBLAS library.</b>
 * @param { module:analytics~logisticRegParam | module:fs.FIn} [arg] - Construction arguments. There are two ways of constructing:
* <br>1. Using the {@link  module:analytics~logisticRegParam} object,
* <br>2. using the file input stream {@link module:fs.FIn}.
 * @example
 * // import analytics module
 * var analytics = require('qminer').analytics;
 * // create the Logistic Regression model
 * var logreg = new analytics.LogReg({ lambda: 2 });
 */
 exports.LogReg = function (arg) { return Object.create(require('qminer').analytics.LogReg.prototype); }
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
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create a logistic regression model
    * var logreg = new analytics.LogReg({ lambda: 10 });
    * // set the parameters of the model
    * logreg.setParams({ lambda: 1 });
    */
 exports.LogReg.prototype.setParams = function (param) { return Object.create(require('qminer').analytics.LogReg.prototype); }
/**
     * Fits a column matrix of feature vectors `X` onto the response variable `y`.
     * @param {module:la.Matrix} X - the column matrix which stores the feature vectors.
     * @param {module:la.Vector} y - the response variable.
     * @param {number} [eps] - the epsilon used for convergence.
     * @returns {module:analytics.LogReg} Self. The model has been updated.
     * @example
     * // import modules
     * var analytics = require('qminer').analytics;
     * var la = require('qminer').la;
     * // create the logistic regression model
     * var logreg = new analytics.LogReg();
     * // create the input matrix and vector for fitting the model
     * var mat = new la.Matrix([[1, 0, -1, 0], [0, 1, 0, -1]]);
     * var vec = new la.Vector([1, 0, -1, -2]);
     * // if OpenBLAS is used, fit the model
     * if (require('qminer').flags.blas) {
     *     logreg.fit(mat, vec);
     * }
     */
 exports.LogReg.prototype.fit = function (X, y, eps) { return Object.create(require('qminer').analytics.LogReg.prototype); }
/**
     * Returns the expected response for the provided feature vector.
     * @param {module:la.Vector} x - the feature vector.
     * @returns {number} the expected response.
     * @example
     * // import modules
     * var analytics = require('qminer').analytics;
     * var la = require('qminer').la;
     * // create the logistic regression model
     * var logreg = new analytics.LogReg();
     * // create the input matrix and vector for fitting the model
     * var mat = new la.Matrix([[1, 0, -1, 0], [0, 1, 0, -1]]);
     * var vec = new la.Vector([1, 0, -1, -2]);
     * // if openblas is used, fit the model and predict the value
     * if (require('qminer').flags.blas) {
     *     // fit the model
     *     logreg.fit(mat, vec);
     *     // create the vector for the prediction
     *     var test = new la.Vector([1, 1]);
     *     // get the prediction
     *     var prediction = logreg.predict(test);
     * };
     */
 exports.LogReg.prototype.predict = function (x) { return 0.0; } 
/**
     * Gives the weights of the model. Type {@link module:la.Vector}.
     * @example 
     * // import modules
     * var analytics = require('qminer').analytics;
     * var la = require('qminer').la;
     * // create the logistic regression model
     * var logreg = new analytics.LogReg();
     * // get the weights of the model
     * var weights = logreg.weights;
     */
 exports.LogReg.prototype.weights = Object.create(require('qminer').la.vector.prototype);
/**
     * Saves the model into the output stream.
     * @param {module:fs.FOut} fout - the output stream.
     * @returns {module:fs.FOut} The output stream `fout`.
     * @example
     * // import modules
     * var analytics = require('qminer').analytics;
     * var la = require('qminer').la;
     * var fs = require('qminer').fs;
     * // create the logistic regression model
     * var logreg = new analytics.LogReg();
     * // create the input matrix and vector for fitting the model
     * var mat = new la.Matrix([[1, 0, -1, 0], [0, 1, 0, -1]]);
     * var vec = new la.Vector([1, 0, -1, -2]);
     * // if openblas is used, fit the model
     * if (require('qminer').flags.blas) {
     *     logreg.fit(mat, vec);
     * };
     * // create an output stream object and save the model
     * var fout = fs.openWrite('logreg_example.bin');
     * logreg.save(fout);
     * fout.close();
     * // create input stream
     * var fin = fs.openRead('logreg_example.bin');
     * // create a Logistic Regression object that loads the model and parameters from input stream
     * var logreg2 = new analytics.LogReg(fin);
     */
 exports.LogReg.prototype.save = function (fout) { return Object.create(require('qminer').fs.FOut.prototype); }
/**
* @typedef {Object} hazardModelParam
* An object used for the construction of {@link module:analytics.PropHazards}.
* @property {number} [lambda = 0] - The regularization parameter.
*/
/**
 * Proportional Hazards Model
 * @class
 * @classdesc Proportional Hazards Model  with a constant hazard function. Uses Newtons method to compute the weights.
 * <b>Before use: QMiner must be built with the OpenBLAS library.</b>
 * @param {module:analytics~hazardModelParam | module:fs.FIn} [arg] - Construction arguments. There are two ways of constructing:
* <br>1. Using the {@link module:analytics~hazardModelParam} object,
* <br>2. using the file input stream {@link module:fs.FIn}.
 * @example
 * // import analytics module
 * var analytics = require('qminer').analytics;
 * // create a Proportional Hazard model
 * var hazard = new analytics.PropHazards();
 */
 exports.PropHazards = function (arg) { return Object.create(require('qminer').analytics.PropHazards.prototype); }
/**
    * Gets the parameters of the model.
    * @returns {module:analytics~hazardModelParam} The parameters of the model.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create a Proportional Hazard model
    * var hazard = new analytics.PropHazards({ lambda: 5 });
    * // get the parameters of the model
    * var param = hazard.getParams();
    */
 exports.PropHazards.prototype.getParams = function () { return { lambda: 0.0 }; }
/**
    * Sets the parameters of the model.
    * @param {module:analytics~hazardModelParam} params - The parameters given to the model.
    * @returns {module:analytics.PropHazards} Self. The model parameters have been updated.
    * @example 
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create a Proportional Hazard model
    * var hazard = new analytics.PropHazards({ lambda: 5 });
    * // set the parameters of the model
    * hazard.setParams({ lambda: 10 });
    */
 exports.PropHazards.prototype.setParams = function (params) { return Object.create(require('qminer').analytics.PropHazards.prototype); }
/**
     * Fits a column matrix of feature vectors `X` onto the response variable `y`.
     * @param {module:la.Matrix} X - The column matrix which stores the feature vectors.
     * @param {module:la.Vector} y - The response variable.
     * @param {number} [eps] - The epsilon used for convergence.
     * @returns {module:analytics.PropHazards} Self. The model has been updated.
     * @example
     * // import modules
     * var analytics = require('qminer').analytics;
     * var la = require('qminer').la;
     * // create the Proportional Hazards model
     * var hazards = new analytics.PropHazards();
     * // create the input matrix and vector for fitting the model
     * var mat = new la.Matrix([[1, 0, -1, 0], [0, 1, 0, -1]]);
     * var vec = new la.Vector([1, 0, -1, -2]);
     * // if openblas used, fit the model
     * if (require('qminer').flags.blas) {
     *     hazards.fit(mat, vec);
     * };
     */
 exports.PropHazards.prototype.fit = function(X, y, eps) { return Object.create(require('qminer').analytics.PropHazards.prototype); }
/**
     * Returns the expected response for the provided feature vector.
     * @param {module:la.Vector} x - The feature vector.
     * @returns {number} The expected response.
     * @example
     * // import modules
     * var analytics = require('qminer').analytics;
     * var la = require('qminer').la;
     * // create the Proportional Hazards model
     * var hazards = new analytics.PropHazards();
     * // create the input matrix and vector for fitting the model
     * var mat = new la.Matrix([[1, 1], [1, -1]]);
     * var vec = new la.Vector([3, 3]);
     * // if openblas used, fit the model and get the prediction
     * if (require('qminer').flags.blas) {
     *     // fit the model
     *     hazards.fit(mat, vec);       
     *     // create a vector for the prediction
     *      var test = new la.Vector([1, 2]);
     *     // predict the value
     *     var prediction = hazards.predict(test);
     * };
     */
 exports.PropHazards.prototype.predict = function(x) { return 0.0; }
/**
     * The models weights. Type {@link module:la.Vector}.
     * @example
     * // import modules
     * var analytics = require('qminer').analytics;
     * var la = require('qminer').la;
     * // create the Proportional Hazards model
     * var hazards = new analytics.PropHazards();
     * // get the weights
     * var weights = hazards.weights;
     */
 exports.PropHazards.prototype.weights = Object.create(require('qminer').la.Vector.prototype);
/**
     * Saves the model into the output stream.
     * @param {module:fs.FOut} fout - The output stream.
     * @returns {module:fs.FOut} The output stream `fout`.
     * @example
     * // import modules
     * var analytics = require('qminer').analytics;
     * var la = require('qminer').la;
     * var fs = require('qminer').fs;
     * // create the Proportional Hazards model
     * var hazards = new analytics.PropHazards();
     * // create the input matrix and vector for fitting the model
     * var mat = new la.Matrix([[1, 0, -1, 0], [0, 1, 0, -1]]);
     * var vec = new la.Vector([1, 0, -1, -2]);
     * // if openblas used, fit the model
     * if (require('qminer').flags.blas) {
     *     hazards.fit(mat, vec);
     * };
     * // create an output stream and save the model
     * var fout = fs.openWrite('hazards_example.bin');
     * hazards.save(fout);
     * fout.close();
     * // create input stream
     * var fin = fs.openRead('hazards_example.bin');
     * // create a Proportional Hazards object that loads the model and parameters from input stream
     * var hazards2 = new analytics.PropHazards(fin);    
     */
 exports.PropHazards.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); }
/**
* @typedef {Object} nnetParam
* An object used for the construction of {@link module:analytics.NNet}.
* @property {Array.<number>} [layout = [1, 2, 1]] - The array representing the network schema.
* @property {number} [learnRate = 0.1] - The learning rate.
* @property {number} [momentum = 0.5] - The momentum of optimization.
* @property {string} [tFuncHidden = 'tanHyper'] - Type of activation function used on hidden nevrons. Possible options are `'tanHyper'`, `'sigmoid'`, `'fastTanh'`, `'softPlus'`, `'fastSigmoid'` and `'linear'`.
* @property {string} [tFuncOut = 'tanHyper'] - Type of activation function used on output nevrons. Possible options are `'tanHyper'`, `'sigmoid'`, `'fastTanh'`, `'softPlus'`, `'fastSigmoid'` and `'linear'`.
*/
/**
* Neural Network Model.
* @class
* @classdesc Holds online/offline neural network model.
* @param {module:analytics~nnetParam | module:fs.FIn} [arg] - Construction arguments. There are two ways of constructing:
* <br>1. Using the {@link module:analytics~nnetParam} object,
* <br>2. using the file input stream {@link module:fs.FIn}.
* @example
* // import module
* var analytics = require('qminer').analytics;
* // create a new Neural Networks model
* var nnet = new analytics.NNet({ layout: [3, 5, 2], learnRate: 0.2, momentum: 0.6 });
*/
 exports.NNet = function (arg) { return Object.create(require('qminer').analytics.NNet.prototype); }
/**
    * Get the parameters of the model.
    * @returns {module:analytics~nnetParam} The constructor parameters.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create a Neural Networks model
    * var nnet = new analytics.NNet();
    * // get the parameters
    * var params = nnet.getParams();
    */
 exports.NNet.prototype.getParams = function () { return { layout: Object.create(require('qminer').la.IntVector.prototype), learnRate: 0.0, momentum: 0.0, tFuncHidden: "", TFuncOut: "" }; }
/**
    * Sets the parameters of the model.
    * @params {module:analytics~nnetParam} params - The given parameters.
    * @returns {module:analytics.NNet} Self. The model parameters have been updated.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create a Neural Networks model
    * var nnet = new analytics.NNet();
    * // set the parameters
    * nnet.setParams({ learnRate: 1, momentum: 10, layout: [1, 4, 3] });
    */
 exports.NNet.prototype.setParams = function (params) { return Object.create(require('qminer').analytics.NNet.prototype); }
/**
    * Fits the model.
    * @param {module:la.Vector | module:la.Matrix} X - The input data.
    * @param {module:la.Vector | module:la.Matrix} Y - The output data.
    * <br> If `X` and `Y` are both {@link module:la.Vector}, then the fitting is in online mode.
    * <br> If `X` and `Y` are both {@link module:la.Matrix}, then the fitting is in batch mode.
    * @returns {module:analytics.NNet} Self. The model has been updated.
    * @example
    * // import modules
    * var analytics = require('qminer').analytics;
    * var la = require('qminer').la;
    * // create a Neural Networks model
    * var nnet = new analytics.NNet({ layout: [2, 3, 4] });
    * // create the matrices for the fitting of the model
    * var matIn = new la.Matrix([[1, 0], [0, 1]]);
    * var matOut = new la.Matrix([[1, 1], [1, 2], [-1, 8], [-3, -3]]);
    * // fit the model
    * nnet.fit(matIn, matOut);
    */
 exports.NNet.prototype.fit = function (input, output) { return Object.create(require('qminer').analytics.NNet.prototype); }
/**
    * Gets the prediction of the vector.
    * @param {module:la.Vector} vec - The prediction vector.
    * @returns {module:la.Vector} The prediction of vector `vec`.
    * @example
    * // import modules
    * var analytics = require('qminer').analytics;
    * var la = require('qminer').la;
    * // create a Neural Networks model
    * var nnet = new analytics.NNet({ layout: [2, 3, 4] });
    * // create the matrices for the fitting of the model
    * var matIn = new la.Matrix([[1, 0], [0, 1]]);
    * var matOut = new la.Matrix([[1, 1], [1, 2], [-1, 8], [-3, -3]]);
    * // fit the model
    * nnet.fit(matIn, matOut);
    * // create the vector for the prediction
    * var test = new la.Vector([1, 1]);
    * // predict the value of the vector
    * var prediction = nnet.predict(test);
    */
 exports.NNet.prototype.predict = function (vec) { return Object.create(require('qminer').la.Vector.prototype); }
/**
    * Saves the model.
    * @param {module:fs.FOut} fout - The output stream.
    * @returns {module:fs.FOut} The output stream `fout`.
    * @example
    * // import modules
    * var analytics = require('qminer').analytics;
    * var la = require('qminer').la;
    * var fs = require('qminer').fs;
    * // create a Neural Networks model
    * var nnet = new analytics.NNet({ layout: [2, 3, 4] });
    * // create the matrices for the fitting of the model
    * var matIn = new la.Matrix([[1, 0], [0, 1]]);
    * var matOut = new la.Matrix([[1, 1], [1, 2], [-1, 8], [-3, -3]]);
    * // fit the model
    * nnet.fit(matIn, matOut);
    * // create an output stream object and save the model
    * var fout = fs.openWrite('nnet_example.bin');
    * nnet.save(fout);
    * fout.close();
    * // load the Neural Network model from the binary
    * var fin = fs.openRead('nnet_example.bin');
    * var nnet2 = new analytics.NNet(fin);
    */
 exports.NNet.prototype.save = function (fout) { return Object.create(require('qminer').fs.FOut.prototype); } 
/**
* @typedef {Object} tokenizerParam
* An object used for the construction of {@link module:analytics.Tokenizer}.
* @property {string} [type='unicode'] - The type of the tokenizer. The different types are: 
*<br>1. 'simple' - Creates break on white spaces.
*<br>2. 'html' - Creates break on white spaces and ignores html tags.
*<br>3. 'unicode' - Creates break on white spaces and normalizes unicode letters, e.g. o?=o?= changes to cso?=z.
*/
/**
 * Tokenizer
 * @class 
 * @classdesc Breaks text into tokens (i.e. words).
 * @param {module:analytics~tokenizerParam} [arg] - Construction arguments. If arg is not given it uses the `'unicode'` tokenizer type.
 * @example
 * // import analytics module
 * var analytics = require('qminer').analytics;
 * // construct Tokenizer object
 * var tokenizer = new analytics.Tokenizer({ type: "simple" });
 */
 exports.Tokenizer = function (arg) { return Object.create(require("qminer").analytics.Tokenizer.prototype); }
/**
    * Tokenizes given string.
    * @param {String} str - String given to tokenize.
    * @returns {Array.<String>} Array of tokens. The number of tokens is equal to number of words in input `str`.
    * Only keeps words, skips all punctuation.
    * Tokenizing contractions (i.e. don't) depends on which type you use. Example: type `'html'` breaks contractions into 2 tokens.
    * @example
    * // import modules
    * var analytics = require('qminer').analytics;
    * var la = require('qminer').la;
    * // construct model
    * var tokenizer = new analytics.Tokenizer();
    * // string you wish to tokenize
    * var string = "What a beautiful day!";
    * // tokenize string using getTokens
    * var tokens = tokenizer.getTokens(string);
    * // output:
    * tokens = ["What", "a", "beautiful", "day"];
    */
 exports.Tokenizer.prototype.getTokens = function (str) { return [""]; }
/**
    * Breaks string into sentences.
    * @param {String} str - String given to break into sentences.
    * @returns {Array.<String>} Array of sentences. The number of sentences is equal to number of sentences in input `str`.
    * How function breaks sentences depends on where you use a full-stop, exclamation mark, question mark or the new line command.
    * Careful: the space between the lines is not ignored. 
    * @example
    * // import modules
    * var analytics = require('qminer').analytics;
    * var la = require('qminer').la;
    * // construct model
    * var tokenizer = new analytics.Tokenizer();
    * // string you wish to tokenize
    * var string = "C++? Alright. Let's do this!";
    * // tokenize text using getSentences
    * var tokens = tokenizer.getSentences(string);
    * // output:
    * tokens = ["C++", " Alright", " Let's do this"];
    */
 exports.Tokenizer.prototype.getSentences = function (str) { return [""]; }
/**
    * Breaks string into paragraphs.
    * @param {String} str - String given to break into paragraphs.
    * @returns {Array.<String>} Array of paragraphs. The number of paragraphs is equal to number of paragraphs in input `str`.
    * When function detects escape sequences `'\n'`, `'\r'` or `'\t'` it breaks text as new paragraph.
    * @example
    * // import modules
    * var analytics = require('qminer').analytics;
    * var la = require('qminer').la;
    * // construct model
    * var tokenizer = new analytics.Tokenizer();
    * // string you wish to tokenize
    * var string = "Yes!\t No?\n Maybe...";
    * // tokenize text using getParagraphs
    * var tokens = tokenizer.getParagraphs(string);
    * // output:
    * tokens = ["Yes", " No", " Maybe"];
    */
 exports.Tokenizer.prototype.getParagraphs = function (str) { return [""]; }
/**
* @typedef {Object} MDSParam
* An object used for the construction of {@link module:analytics.MDS}.
* @property {number} [maxSecs=500] - The maximum time period to compute Multidimensional Scaling of a matrix.
* @property {number} [maxStep=5000] - The maximum number of iterations.
* @property {number} [minDiff=1e-4] - The minimum difference criteria in MDS.
* @property {string} [distType="Euclid"] - The type of distance used. Available types: "Euclid", "Cos", "SqrtCos".
*/
/**
* Multidimensional Scaling
* @class
* @classdesc Scales a higher level vectors into a 2D vector space such that the distances between vectors are preserved as well as possible.
* @param {module:analytics~MDSParam | module:fs.FIn} [arg] - Construction arguments. There are two ways of constructing:
* <br>1. Using the {@link module:analytics~MDSParam} object,
* <br>2. using the file input stream {@link module:fs.FIn}.
* @example
* // import analytics module
* var analytics = require('qminer').analytics;
* // construct a MDS instance
* var mds = new analytics.MDS({ maxStep: 300, distType: 'Cos' });
*/
 exports.MDS = function (arg) { return Object.create(require('qminer').analytics.MDS.prototype); }
/**
    * Get the parameters.
    * @returns {module:analytics~MDSParam} The constructor parameters.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create a MDS instance
    * var mds = new analytics.MDS();
    * // get the (default) parameters of the instance
    * // returns { maxStep: 5000, maxSecs: 300, minDiff: 1e-4, distType: "Euclid" }
    * var params = mds.getParams();
    */
 exports.MDS.prototype.getParams = function () { return { maxStep: 0, maxSecs: 0, minDiff: 0, distType: "" }; }
/**
    * Set the parameters.
    * @param {module:analytics~MDSParam} params - The constructor parameters.
    * @returns {module:analytics.MDS} Self. The model parameters have been updated.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create a MDS instance
    * var mds = new analytics.MDS();
    * // get the (default) parameters of the instance
    * // returns { maxStep: 5000, maxSecs: 300, minDiff: 1e-4, distType: "Euclid" }
    * var params = mds.getParams();
    */
 exports.MDS.prototype.setParams = function (params) { return Object.create(require('qminer').analytics.MDS.prototype); }
/**
    * Get the MDS of the given matrix.
    * @param {module:la.Matrix | module:la.SparseMatrix} mat - The multidimensional matrix.
    * @param {function} [callback] - The callback function receiving the error parameter (`err`) and the result parameter (`res`).
    * <i>Only for the asynchronous function.</i>
    * @returns {module:la.Matrix} The matrix of dimensions `mat.cols` x 2, where the i-th row of the matrix is the 2D representation 
    * of the i-th column of `mat`.
    * @example <caption>Asynchronous function</caption>
    * // import the modules
    * var analytics = require('qminer').analytics;
    * var la = require('qminer').la;
    * // create a MDS instance
    * var mds = new analytics.MDS();
    * // create the multidimensional matrix
    * var mat = new la.Matrix({ rows: 50, cols: 10, random: true });
    * // get the 2d representation of mat 
    * mds.fitTransformAsync(mat, function (err, res) {
    *    if (err) throw err;
    *    // successful calculation
    *    var mat2d = res;
    * }); 
    * @example <caption>Synchronous function</caption>
    * // import the modules
    * var analytics = require('qminer').analytics;
    * var la = require('qminer').la;
    * // create a MDS instance
    * var mds = new analytics.MDS();
    * // create the multidimensional matrix
    * var mat = new la.Matrix({ rows: 50, cols: 10, random: true });
    * // get the 2d representation of mat 
    * var mat2d = mds.fitTransform(mat); 
    */
 exports.MDS.prototype.fitTransform = function (mat, callback) { return Object.create(require('qminer').la.Matrix.prototype); }
/**
    * Save the MDS model.
    * @param {module:fs.FOut} fout - The output stream.
    * @returns {module:fs.FOut} The output stram `fout`.
    * @example
    * // import modules
    * var analytics = require('qminer').analytics;
    * var fs = require('qminer').fs;
    * // create a MDS instance
    * var mds = new analytics.MDS({ iter: 200, MaxStep: 10 });
    * // create the file output stream
    * var fout = new fs.openWrite('MDS.bin');
    * // save the MDS instance
    * mds.save(fout);
    * fout.close();
    * // load the MDS instance
    * var fin = fs.openRead('MDS.bin');
    * var mds2 = new analytics.MDS(fin);
    */
 exports.MDS.prototype.save = function (fout) { return Object.create(require('qminer').fs.FOut.prototype); }
/**
* @typedef {Object} KMeansParam
* An object used for the construction of {@link module:analytics.KMeans}.
* @property {number} [iter=10000] - The maximum number of iterations.
* @property {number} [k=2] - The number of centroids.
* @property {boolean} [allowEmpty=true] - Whether to allow empty clusters to be generated.
* @property {string} [centroidType="Dense"] - The type of centroids. Possible options are `'Dense'` and `'Sparse'`.
* @property {string} [distanceType="Euclid"] - The distance type used at the calculations. Possible options are `'Euclid'` and `'Cos'`.
* @property {boolean} [verbose=false] - If `false`, the console output is supressed.
* @property {Array.<number>} [fitIdx] - The index array used for the construction of the initial centroids.
* @property {Object} [fitStart] - The KMeans model returned by {@link module:analytics.KMeans.prototype.getModel} used for centroid initialization.
* @property {(module:la.Matrix | module:la.SparseMatrix)} fitStart.C - The centroid matrix.
*/
/** 
* KMeans Clustering 
* @classdesc KMeans Clustering is an iterative, data-partitioning algorithm that assigns observations into K clusters.
* @class
* @param {module:analytics~KMeansParam | module:fs.FIn} [arg] - Construction arguments. There are two ways of constructing:
* <br>1. Using the {@link module:analytics~KMeansParam} object,
* <br>2. using the file input stream {@link module:fs.FIn}.
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
 exports.KMeans = function (arg) { return Object.create(require('qminer').analytics.KMeans.prototype); }
/**
    * Returns the parameters.
    * @returns {module:analytics~KMeansParam} The construction parameters.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create a new KMeans object
    * var KMeans = new analytics.KMeans({ iter: 1000, k: 5 });
    * // get the parameters
    * var json = KMeans.getParams();
    */
 exports.KMeans.prototype.getParams = function () { return { iter: 10000, k: 2, distanceType: "Euclid", centroidType: "Dense", verbose: false }; }
/**
     * Sets the parameters.
     * @param {module:analytics~KMeansParam} params - The construction parameters.
     * @returns {module:analytics.KMeans} Self. The model parameters have been updated.
     * @example
     * // import analytics module
     * var analytics = require('qminer').analytics;
     * // create a new KMeans object
     * var KMeans = new analytics.KMeans();
     * // change the parameters of the KMeans object
     * KMeans.setParams({ iter: 1000, k: 5 });
     */
 exports.KMeans.prototype.setParams = function (params) { return Object.create(require('qminer').analytics.KMeans.prototype); }
/**
     * Calculates the centroids.
     * @param {module:la.Matrix | module:la.SparseMatrix} X - Matrix whose columns correspond to examples.
     * @returns {module:analytics.KMeans} Self. The model has been updated.
     * @example <caption> Asynchronous function </caption>
     * // import analytics module
     * var analytics = require('qminer').analytics;
     * var la = require('qminer').la;
     * // create a new KMeans object
     * var KMeans = new analytics.KMeans({ iter: 1000, k: 3 });
     * // create a matrix to be fitted
     * var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
     * // create the model with the matrix X
     * KMeans.fitAsync(X, function (err) {
     *     if (err) console.log(err);
     *     // successful calculation
     * });
     *
     * @example <caption> Synchronous function </caption>
     * var analytics = require('qminer').analytics;
     * var la = require('qminer').la;
     * // create a new KMeans object
     * var KMeans = new analytics.KMeans({ iter: 1000, k: 3 });
     * // create a matrix to be fitted
     * var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
     * // create the model with the matrix X
     * KMeans.fit(X);
     */
 exports.KMeans.prototype.fit = function (X) { return Object.create(require('qminer').analytics.KMeans.prototype); }
/**
     * Returns an vector of cluster id assignments.
     * @param {module:la.Matrix | module:la.SparseMatrix} A - Matrix whose columns correspond to examples.
     * @returns {module:la.IntVector} Vector of cluster assignments.
     * @example
     * // import analytics module
     * var analytics = require('qminer').analytics;
     * var la = require('qminer').la;
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
 exports.KMeans.prototype.predict = function (A) { return Object.create(require('qminer').la.IntVector.prototype); }
/**
     * Transforms the points to vectors of distances to centroids.
     * @param {module:la.Matrix | module:la.SparseMatrix} A - Matrix whose columns correspond to examples.
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
 exports.KMeans.prototype.transform = function (A) { return Object.create(require('qminer').la.Matrix.prototype); }
/**
     * Permutates the clusters, and with it {@link module:analytics.KMeans#centroids}, {@link module:analytics.KMeans#medoids} and {@link module:analytics.KMeans#idxv}.
     * @param {module:la.IntVector} mapping - The mapping, where `mapping[4] = 2` means "map cluster 4 into cluster 2".
     * @returns {module:analytics.KMeans} Self. The clusters has been permutated.
     * @example 
     * // import the modules
     * var analytics = require('qminer').analytics;
     * var la = require('qminer').la;
     * // create a new KMeans object
     * var KMeans = new analytics.KMeans({ iter: 1000, k: 3 });
     * // create a matrix to be fitted
     * var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
     * // create the model with the matrix X
     * KMeans.fit(X);
     * // create the mapping vector
     * var Mapping = new la.IntVector([1, 0, 2]);
     * // permutate the clusters.
     * KMeans.permuteCentroids(Mapping);
     */
 exports.KMeans.prototype.permuteCentroids = function (mapping) { return Object.create(require('qminer').analytics.KMeans.prototype); }
/**
     * Saves KMeans internal state into (binary) file.
     * @param {module:fs.FOut} fout - The output stream.
     * @returns {module:fs.FOut} The output stream `fout`.
     * @example
     * // import the modules
     * var analytics = require('qminer').analytics;
     * var la = require('qminer').la;
     * var fs = require('qminer').fs;
     * // create a new KMeans object
     * var KMeans = new analytics.KMeans({ iter: 1000, k: 3 });
     * // create a matrix to be fitted
     * var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
     * // create the model with the matrix X
     * KMeans.fit(X); 
     * // create the file output stream
     * var fout = new fs.openWrite('KMeans.bin');
     * // save the KMeans instance
     * KMeans.save(fout);
     * fout.close();
     * // load the KMeans instance
     * var fin = fs.openRead('KMeans.bin');
     * var KMeans2 = new analytics.KMeans(fin);
     */
 exports.KMeans.prototype.save = function (fout) { return Object.create(require('qminer').fs.FOut.prototype); }
/**
     * The centroids created with the fit method. Type {@link module:la.Matrix}.
     * @example
     * // import the modules
     * var analytics = require('qminer').analytics;
     * // create a new KMeans object
     * var KMeans = new analytics.KMeans({ iter: 1000, k: 3 });
     * // get the centroids
     * var centroids = KMeans.centroids;
     */
 exports.KMeans.prototype.centroids = Object.create(require('qminer').la.Matrix.prototype);
/**
    * The medoids created with the fit method. Type {@link module:la.IntVector}.
    * @example
    * // import the modules
    * var analytics = require('qminer').analytics;
    * // create a new KMeans object
    * var KMeans = new analytics.KMeans({ iter: 1000, k: 3 });
    * // get the centroids
    * var medoids = KMeans.medoids;
    */
 exports.KMeans.prototype.medoids = Object.create(require('qminer').la.IntVector.prototype);
/**
    * The integer vector containing the cluster ids of the training set created with the fit method. Type {@link module:la.IntVector}.
    * @example
    * // import the modules
    * var analytics = require('qminer').analytics;
    * // create a new KMeans object
    * var KMeans = new analytics.KMeans({ iter: 1000, k: 3 });
    * // get the idxv
    * var idxv = KMeans.idxv;
    */
 exports.KMeans.prototype.idxv = Object.create(require('qminer').la.IntVector.prototype);
/**
* @typedef {Object} RecSysParam
* An object used for the construction of {@link module:analytics.RecommenderSys}.
* @property {number} [iter=10000] - The maximum number of iterations.
* @property {number} [k=2] - The number of centroids.
* @property {number} [tol=1e-3] - The tolerance.
* @property {boolean} [verbose=false] - If false, the console output is supressed.
*/
/**
* Recommender System
* @classdesc The recommender system algorithm using Weighted Non-negative Matrix Factorization to predict the 
* unknown values. If `A` is a matrix with unknown values it calculates the matrices `U` and `V` such that `U*V` approximates `A`.
* @class
* @param {module:analytics~RecSysParam | module:fs.FIn} [arg] - Construction arguments. There are two ways of constructing:
* <br>1. Using the {@link module:analytics~RecSysParam} object,
* <br>2. using the file input stream {@link module:fs.FIn}.
* @example
* // import analytics and la modules
* var analytics = require('qminer').analytics;
* var la = require('qminer').la;
* // create a Recommender System object
* var recSys = new analytics.RecommenderSys({ tol: 1e-3, iter: 10000, k: 2, verbose: false });
* // create the matrix to be fitted
* var X = new la.Matrix([[1, 2, 1], [1, 1, 3]]);
* // create the model
* recSys.fit(X);
*/
 exports.RecommenderSys = function (arg) { return Object.create(require('qminer').analytics.RecommenderSys.prototype); }
/**
    * Returns the parameters.
    * @returns {module:analytics~RecSysParam} The construction parameters.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create a new KMeans object
    * var recSys = new analytics.RecommenderSys({ iter: 1000, k: 5 });
    * // get the parameters
    * var json = recSys.getParams();
    */
 exports.RecommenderSys.prototype.getParams = function () { return { iter: 10000, k: 2, tol: 1e-3, verbose: false }; }
/**
    * Sets the parameters.
    * @param {module:analytics~RecSysParam} params - The construction parameters.
    * @returns {module:analytics.RecommenderSys} Self. The parameters has been updated.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create a new Recommender System object
    * var recSys = new analytics.RecommenderSys();
    * // change the parameters of the Recommender System object
    * recSys.setParams({ iter: 1000, k: 5 });
    */
 exports.RecommenderSys.prototype.setParams = function (params) { return Object.create(require('qminer').analytics.RecommenderSys.prototype); }
/**
     * Gets the model.
     * @returns {Object} An object `recSysModel` containing the properties:
     * <br>1. `recSysModel.U` - The matrix `U` from the weighted NMF. Type {@link module:la.Matrix}.
     * <br>2. `recSysModel.V` - The matrix `V` from the weighted NMF. Type {@link module:la.Matrix}.
     * @example
     * // import modules
     * //var analytics = require('qminer').analytics;
     * //var la = require('qminer').la;
     * // create a new Recommender System object
     * //var recSys = new analytics.RecommenderSys({ iter: 1000, k: 3 });
     * // create a matrix to be fitted
     * //var X = new la.Matrix([[1, 5, 0], [1, 0, 3]]);
     * // create the model with the matrix X
     * //recSys.fit(X);
     * // get the model
     * //var model = recSys.getModel();
     */
 exports.RecommenderSys.prototype.getModel = function () { return { U: Object.create(require('qminer').la.Matrix.prototype), V: Object.create(require('qminer').la.Matrix.prototype) }; }
/**
    * Fits the input matrix to the recommender model.
    * @param {module:la.Matrix | module:la.SparseMatrix} A - Matrix with the ratings, where it A_ij element is the rating that the i-th person
    * gave to the j-th item. If A_ij = 0, the data doesn't exist.
    * @returns {module:analytics.RecommenderSys} Self. The model has been fitted.
    * @example <caption> Asynhronous function </caption>
    * // import modules
    * var analytics = require('qminer').analytics;
    * var la = require('qminer').la;
    * // create a new Recommender System object
    * var recSys = new analytics.RecommenderSys({ iter: 1000, k: 2 });
    * // create a matrix to be fitted
    * var X = new la.Matrix([[1, 5, 0], [1, 0, 3]]);
    * // create the model with the matrix X
    * recSys.fitAsync(X, function (err) {
    *    if (err) { console.log(err); }
    *    // successful calculation
    * });
    * @example <caption> Synhronous function </caption>
    * // import modules
    * var analytics = require('qminer').analytics;
    * var la = require('qminer').la;
    * // create a new Recommender System object
    * var recSys = new analytics.RecommenderSys({ iter: 1000, k: 2 });
    * // create a matrix to be fitted
    * var X = new la.Matrix([[1, 5, 0], [1, 0, 3]]);
    * // create the model with the matrix X
    * recSys.fit(X);
    */
 exports.RecommenderSys.prototype.fit = function (A) { return Object.create(require('qminer').analytics.RecommenderSys.prototype); }
/**
    * Saves RecommenderSys internal state into (binary) file.
    * @param {module:fs.FOut} fout - The output stream.
    * @returns {module:fs.FOut} The output stream `fout`.
    * @example
    * // import modules
    * var analytics = require('qminer').analytics;
    * var fs = require('qminer').fs;
    * // create a new Recommender System object
    * var recSys = new analytics.RecommenderSys();
    * // change the parameters of the Recommender System object
    * recSys.setParams({ iter: 1000, k: 5 });
    * // create the file output stream
    * var fout = new fs.openWrite('recsys.bin');
    * // save the RecommenderSys instance
    * recSys.save(fout);
    * fout.close();
    * // load the RecommenderSys instance
    * var fin = fs.openRead('recsys.bin');
    * var recSys2 = new analytics.RecommenderSys(fin);
    */
 exports.RecommenderSys.prototype.save = function (fout) { return Object.create(require('qminer').fs.FOut.prototype); }
 exports.GraphCascade = function (arg) { return Object.create(require('qminer').analytics.GraphCascade.prototype); }
/**
    * Sets the cascade time for a given node
    * @param {string} nodeId - 
    * @param {number} timestamp - 
    */
/**
    * Computes the posterior for timestamps of unobserved nodes
    * @param {number} timestamp - current time
    */
/**
    * Returns the posteriors
    * @returns {Object} - model
    */
/**
    * Returns the pruned directed acyclic graph
    * @returns {Object} - dag
    */


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
    exports.SVC.prototype.getModel = function() { return { weights: this.weights }; }
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
    exports.SVR.prototype.getModel = function() { return { weights: this.weights }; }

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
    * var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 12000 }, cats: 2 });
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
        * @param {module:analytics~OneVsAllParam} params - The constructor parameters.
        * @returns {module:analytics.OneVsAll} Self. The parameters are changed.
        * @example
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // create a new OneVsAll object with the model analytics.SVC
        * var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 12000 }, cats: 2 });
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
         * var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 12000 }, cats: 2 });
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
         * be integer numbers between `0` and `oneVsAllParam.categories-1`.
         * @returns {module:analytics.OneVsAll} Self. The models have been fitted.
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

    

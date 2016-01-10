/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef ANALYTICS_H_
#define ANALYTICS_H_

#include <node.h>
#include <node_object_wrap.h>
#include "../nodeutil.h"
#include "fs_nodejs.h"
#include "la_nodejs.h"
#include "qminer_ftr.h"
#include "../../glib/mine/mine.h"

/**
 * Analytics module.
 * @module analytics
 * @example
 * // import module, load dataset, create model, evaluate model
 */

///////////////////////////////
// QMiner-JavaScript-Support-Vector-Machine-Model
// Holds SVM classification or regression model. 
class TNodeJsSvmModel : public node::ObjectWrap {
	friend class TNodeJsUtil;
	friend class TNodeJsSVC;
	friend class TNodeJsSVR;
public:
	static const TStr GetClassId() { return "SvmModel"; }
private:
    // parameters
	TStr Algorithm;	
	double SvmCost;	
	double SvmUnbalance; // classification specific
	double SvmEps; // regression specific
	int SampleSize;
	int MxIter;
	int MxTime;
	double MnDiff;
	bool Verbose;
	PNotify Notify;

	// model
	TSvm::TLinModel Model;

	TNodeJsSvmModel(const PJsonVal& ParamVal);
	TNodeJsSvmModel(TSIn& SIn);

	static TNodeJsSvmModel* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

public:
	//- `params = svmModel.getParams()` -- returns the parameters of this model as a Javascript object
	JsDeclareFunction(getParams);
	//- `svmModel = svmModel.getParams(params)` -- sets one or more parameters given in the input argument `params` returns this
	JsDeclareFunction(setParams);
    //- `vec = svmModel.weights` -- weights of the SVM linear model as a full vector `vec`
	JsDeclareProperty(weights);
    //- `fout = svmModel.save(fout)` -- saves model to output stream `fout`. Returns `fout`.
	JsDeclareFunction(save);
	//- `num = svmModel.decisionFunction(vec)` -- sends vector `vec` through the model and returns the distance to the decision boundery as a real number `num`
    //- `num = svmModel.decisionFunction(spVec)` -- sends sparse vector `spVec` through the model and returns the distance to the decision boundery as a real number `num`
	JsDeclareFunction(decisionFunction);
    //- `num = svmModel.predict(vec)` -- sends vector `vec` through the model and returns the prediction as a real number `num` (-1 or 1 for classification)
    //- `num = svmModel.predict(spVec)` -- sends sparse vector `spVec` through the model and returns the prediction as a real number `num` (-1 or 1 for classification)
    JsDeclareFunction(predict);

private:
	void UpdateParams(const PJsonVal& ParamVal);
	PJsonVal GetParams() const;
	void Save(TSOut& SOut) const;
	void ClrModel();
};

///////////////////////////////
// QMiner-JavaScript-Support-Vector-Classification

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
* var SVC = new analytics.SVC({ verbose: false });
* // train classifier
* SVC.fit(featureMatrix, targets);
* // set up a fake test vector
* var test = new la.Vector([1.1, -0.5]);
* // predict the target value
* var prediction = SVC.predict(test);
*/
//# exports.SVC = function(arg) { return Object.create(require('qminer').analytics.SVC.prototype); };

class TNodeJsSVC : public TNodeJsSvmModel {
public:
	static void Init(v8::Handle<v8::Object> exports);

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
	//# exports.SVC.prototype.getParams = function() { return { algorithm: '', c: 0, j: 0, batchSize: 0, maxIterations: 0, maxTime: 0, minDiff: 0, verbose: true } };

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
	//# exports.SVC.prototype.setParams = function(param) { return Object.create(require('qminer').analytics.SVC.prototype); };

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
	//# exports.SVC.prototype.weights = Object.create(require('qminer').la.Vector.prototype);

	/**
	* Saves model to output file stream.
	* @param {module:fs.FOut} fout - Output stream.
	* @returns {module:fs.FOut} The Output stream.
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
	//# exports.SVC.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); }
    
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
    //# exports.SVC.prototype.decisionFunction = function(X) { return (X instanceof require('qminer').la.Vector | X instanceof require('qminer').la.SparseVector) ? 0 : Object.create(require('qminer').la.Vector.prototype); }
    
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
	//# exports.SVC.prototype.predict = function(X) { return (X instanceof require('qminer').la.Vector | X instanceof require('qminer').la.SparseVector) ? 0 : Object.create(require('qminer').la.Vector.prototype); }
	
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
	//# exports.SVC.prototype.fit = function(X, y) { return Object.create(require('qminer').analytics.SVC.prototype); }
	JsDeclareFunction(fit);
};

///////////////////////////////
// QMiner-JavaScript-Support-Vector-Regression

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
* var SVR = new analytics.SVR({ verbose: false });
* // Train regression
* SVR.fit(featureMatrix, targets);
* // Set up a fake test vector
* var test = new la.Vector([1.1, -0.8]);
* // Predict the target value
* var prediction = SVR.predict(test);
*/
//# exports.SVR = function(arg) { return Object.create(require('qminer').analytics.SVR.prototype); };

class TNodeJsSVR : public TNodeJsSvmModel {
public:
	static void Init(v8::Handle<v8::Object> exports);
    
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
	//# exports.SVR.prototype.getParams = function() { return { c: 0, eps: 0, batchSize: 0, maxIterations: 0, maxTime: 0, minDiff: 0, verbose: true } };

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
	//# exports.SVR.prototype.setParams = function(param) { return Object.create(require('qminer').analytics.SVR.prototype); };

	/**
	* The vector of coefficients of the linear model.
	* @returns {module:la.Vector} weights - Vector of coefficients of the linear model.
	*/
	//# exports.SVR.prototype.weights = Object.create(require('qminer').la.Vector.prototype);

	/**
	* Saves model to output file stream.
	* @param {module:fs.FOut} fout - Output stream.
	* @returns {module:fs.FOut} Output stream.
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
	* var SVR2 = new analytics.SVR()
	*/
	//# exports.SVR.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); }

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
    //# exports.SVR.prototype.decisionFunction = function(X) { return (X instanceof require('qminer').la.Vector | X instanceof require('qminer').la.SparseVector) ? 0 : Object.create(require('qminer').la.Vector.prototype); }

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
	//# exports.SVR.prototype.predict = function(X) { return (X instanceof require('qminer').la.Vector | X instanceof require('qminer').la.SparseVector) ? 0 : Object.create(require('qminer').la.Vector.prototype); }

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
	//# exports.SVR.prototype.fit = function(X, y) { return Object.create(require('qminer').analytics.SVR.prototype); }
	JsDeclareFunction(fit);	
};

/////////////////////////////////////////////
// Ridge Regression

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
 * var regmod = new analytics.RidgeReg({ gamma: 1.0 });
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
 * // true model
 * w.print();
 * // trained model');
 * regmod.weights.print();
 * // cosine between the true and the estimated model should be close to 1 if the fit succeeded
 * var cos = regmod.weights.cosine(w);
 */
//# exports.RidgeReg = function(arg) {};
class TNodeJsRidgeReg : public node::ObjectWrap {
    friend class TNodeJsUtil;
public:
    static void Init(v8::Handle<v8::Object> exports);
    static const TStr GetClassId() { return "RidgeReg"; }
    
private:
    TRegression::TRidgeReg Model;
    
    TNodeJsRidgeReg(TSIn& SIn): Model(SIn) { }
    TNodeJsRidgeReg(const TRegression::TRidgeReg& _Model): Model(_Model) { }
    
    static TNodeJsRidgeReg* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);
    
public:

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
	//# exports.RidgeReg.prototype.getParams = function () { return { gamma: 0.0 } }
	JsDeclareFunction(getParams);

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
	//# exports.RidgeReg.prototype.setParams = function (gamma) { return Object.create(require('qminer').analytics.RidgeReg.prototype); }
	JsDeclareFunction(setParams);

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
    //# exports.RidgeReg.prototype.fit = function(X, y) { return Object.create(require('qminer').analytics.RidgeReg.prototype); }
    JsDeclareFunction(fit);

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
    //# exports.RidgeReg.prototype.decisionFunction = function(X) { return 0.0; }

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
    //# exports.RidgeReg.prototype.predict = function(X) { return 0.0; }
    JsDeclareFunction(predict);
    
    /**
     * @property {module:la.Vector} weights - Vector of coefficients for linear regression.
     */
    //# exports.RidgeReg.prototype.weights = Object.create(require('qminer').la.Vector.prototype);
    JsDeclareProperty(weights);
    
    /**
     * Saves the model into the output stream.
     *
     * @param {module:fs.FOut} fout - Output stream.
	 * @returns {module:fs.FOut} THe output stream fout.
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
    //# exports.RidgeReg.prototype.save = function(fout) { Object.create(require('qminer').fs.FOut.prototype); };
    JsDeclareFunction(save);
};

/////////////////////////////////////////////
// Sigmoid
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
//# exports.Sigmoid = function(arg) {};
class TNodeJsSigmoid : public node::ObjectWrap {
    friend class TNodeJsUtil;
public:
    static void Init(v8::Handle<v8::Object> exports);
    static const TStr GetClassId() { return "Sigmoid"; }
    
private:
    TSigmoid Sigmoid;
    
    TNodeJsSigmoid() {}
    TNodeJsSigmoid(TSIn& SIn): Sigmoid(SIn) {}
    
    static TNodeJsSigmoid* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);
    
public:

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
	//# exports.Sigmoid.prototype.getParams = function () { return {}; }
	JsDeclareFunction(getParams);

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
	//# exports.Sigmoid.prototype.setParams = function (arg) { return Object.create(require('qminer').analytics.Sigmoid.prototype); }
	JsDeclareFunction(setParams);

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
	//# exports.Sigmoid.prototype.getModel = function () {return { A: 0, B: 0 }; }
	JsDeclareFunction(getModel);

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
    //# exports.Sigmoid.prototype.fit = function(X, y) { return Object.create(require('qminer').analytics.Sigmoid.prototype); }
    JsDeclareFunction(fit);
    
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
    //# exports.Sigmoid.prototype.decisionFunction = function(x) { return (x instanceof la.Vector) ? Object.create(require('qminer').la.Vector.prototype) : 0.0; }

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
    //# exports.Sigmoid.prototype.predict = function(x) { return (x instanceof la.Vector) ? Object.create(require('qminer').la.Vector.prototype) : 0.0; }
    JsDeclareFunction(predict);
    
    /**
     * Saves the model into the output stream.
     *
     * @param {module:fs.FOut} fout - Output stream.
	 * @returns {module:fs.FOut} The output stream fout.
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
    //# exports.Sigmoid.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); };
    JsDeclareFunction(save);
};

/////////////////////////////////////////////
// Nearest Neighbor Annomaly Detection

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
 * @param {(module:analytics~detectorParam|module:fs.FIn)} [detectorParam] - Constructor parameters.
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
//# exports.NearestNeighborAD = function(arg) { return Object.create(require('qminer').analytics.NearestNeighborAD.prototype); };
class TNodeJsNNAnomalies : public node::ObjectWrap {
    friend class TNodeJsUtil;
public:
    static void Init(v8::Handle<v8::Object> exports);
    static const TStr GetClassId() { return "NearestNeighborAD"; }
    
private:
    TAnomalyDetection::TNearestNeighbor Model;

    // create from json parameters
    TNodeJsNNAnomalies(const PJsonVal& ParamVal);
    // serialization
    TNodeJsNNAnomalies(TSIn& SIn): Model(SIn) { }
    void Save(TSOut& SOut);
    // change parameters, resets the model
    void SetParams(const PJsonVal& JsonVal);
    // get parameters JSon
    PJsonVal GetParams() const;
    // get model as JSon
    PJsonVal GetModel() const;

    static TNodeJsNNAnomalies* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

public:
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
	//# exports.NearestNeighborAD.prototype.setParams = function (newParams) { return Object.create(require('qminer').analytics.NearestNeighborAD.prototype); }
    JsDeclareFunction(setParams);
    
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
	//# exports.NearestNeighborAD.prototype.getParams = function () { return { rate: 0.0, windowSize: 0.0 }; }
    JsDeclareFunction(getParams);
    
    /**
     * Save model to provided output stream.
     * @param {module:fs.FOut} fout - The output stream.
     * @returns {module:fs.FOut} Provided output stream fout.
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
    //# exports.NearestNeighborAD.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); }
    JsDeclareFunction(save);
    
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
	//# exports.NearestNeighborAD.prototype.getModel = function () { return { threshold: 0.0 }; }
    JsDeclareFunction(getModel);

	/**
	* Adds a new point to the known points and recomputes the threshold.
	* @param {module:la.SparseVector} X - Test example (vector input)
	* @param {number} recId - Integer record ID, used in NearestNeighborAD.explain
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
    //# exports.NearestNeighborAD.prototype.partialFit = function(X) { return Object.create(require('qminer').NearestNeighborAD.prototype); }
    JsDeclareFunction(partialFit);
    
	/**
	* Analyzes the nearest neighbor distances and computes the detector threshold based on the rate parameter.
	* @param {module:la.SparseMatrix} A - Matrix whose columns correspond to known examples. Gets saved as it is part of
	* @param {module:la.IntVector} [idVec] - An integer vector of IDs
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
    //# exports.NearestNeighborAD.prototype.fit = function(A, idVec) { return Object.create(require('qminer').NearestNeighborAD.prototype); }
    JsDeclareFunction(fit);

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
    //# exports.NearestNeighborAD.prototype.decisionFunction = function(x) { return 0.0; }
    JsDeclareFunction(decisionFunction);

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
    //# exports.NearestNeighborAD.prototype.predict = function(x) { return 0.0; }
    JsDeclareFunction(predict);

	/**
	* @typedef {Object} NearestNeighborADExplain
	* A Json object used for interpreting the predictions of {@link module:analytics.NearestNeighborAD}.
	* @param {number} nearestID - The ID of the nearest neighbor
	* @param {Array<number>} featureIDs - the IDs of the features that contributed to the distance score
	* @param {Array<number>} featureContributions - fractions of the contributions of each feature to the total distance (the scores sum to 1.0). The elements correspond to features in the array `featureIDs`
	*/

	/**
	* Returns a JSON object that encodes the ID of the nearest neighbor and the features that contributed to the distance
	* @param {module:la.SparseVector} x - Test vector.
	* @returns {module:analytics~NearestNeighborADExplain} The explanation object
	* @example
	* // import modules
	* var analytics = require('qminer').analytics;
	* var la = require('qminer').la;
	* // create a new NearestNeighborAD object
	* var neighbor = new analytics.NearestNeighborAD({rate:0.05, windowSize:3});
	* // create a new sparse matrix
	* var matrix = new la.SparseMatrix([[[0, 1], [1, 2]], [[0, -2], [1, 3]], [[0, 0], [1, 1]]]);
	* // fit the model with the matrix and provide a vector record IDs
	* neighbor.fit(matrix, new la.IntVector([3541,1112,4244]));
	* // create a new sparse vector
	* var vector = new la.SparseVector([[0, 4], [1, 0]]);
	* // check if the vector is an anomaly
	* var explanation = neighbor.explain(vector); // returns an explanation
	*/
	//# exports.NearestNeighborAD.prototype.explain = function(x) { return {}; }
	JsDeclareFunction(explain);

	/**
	* Returns true when the model has enough data to initialize.
	*/
	//# exports.NearestNeighborAD.prototype.init = false;
	JsDeclareProperty(init);
};

///////////////////////////////
////// code below not yet ported or verified for scikit
///////////////////////////////

///////////////////////////////
// QMiner-JavaScript-Recursive-Linear-Regression

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
* @param {(module:analytics~recLinearRegParam|module:fs.FIn)} param - The constructor parameter json object.
* @example
* // import analytics module
* var analytics = require('qminer').analytics;
* // create the recursive linear regression model holder
* var linreg = new analytics.RecLinReg({ dim: 10, regFact: 1.0, forgetFact: 1.0 });
*/
//# exports.RecLinReg = function(param) { return Object.create(require('qminer').analytics.RecLinReg.prototype); }
class TNodeJsRecLinReg : public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	TSignalProc::PRecLinReg Model;
	TNodeJsRecLinReg(const TSignalProc::PRecLinReg& Model);
public:
	static void Init(v8::Handle<v8::Object> exports);
	static const TStr GetClassId() { return "RecLinReg"; }
private:

	static TNodeJsRecLinReg* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);
	
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
	//# exports.RecLinReg.prototype.partialFit = function (vec, num) { return Object.create(require('qminer').analytics.RecLinReg.prototype); }
	JsDeclareFunction(partialFit);

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
	//# exports.RecLinReg.prototype.fit = function (mat, vec) { return Object.create(require('qminer').analytics.RecLinReg.prototype); }
	JsDeclareFunction(fit);

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
	//# exports.RecLinReg.prototype.predict = function (vec) { return 0.0 }  
	JsDeclareFunction(predict);

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
	//# exports.RecLinReg.prototype.setParams = function (param) { return Object.create(require('qminer').analytics.RecLinReg.prototype); }
	JsDeclareFunction(setParams);

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
	//# exports.RecLinReg.prototype.getParams = function () { return { dim: 0, regFact: 1.0, forgetFact: 1.0 }}
	JsDeclareFunction(getParams);

	/**
	* Gives the weights of the model.
	*/
	//# exports.RecLinReg.prototype.weights = Object.create(require('qminer').la.Vector);
	JsDeclareProperty(weights);

	/**
	* Gets the dimensionality of the model.
	*/
	//# exports.RecLinReg.prototype.dim = 0;
	JsDeclareProperty(dim);

	/**
	* Save model to provided output stream.
	* @param {module:fs.FOut} fout - The output stream.
	* @returns {module:fs.FOut} Provided output stream fout.
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
	//# exports.RecLinReg.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); }
	JsDeclareFunction(save);

private:
	PJsonVal GetParams() const;
};

/////////////////////////////////////////////
// Logistic Regression

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
 * @param {(module:analytics~logisticRegParam|module:fs.FIn)} [opts] - The options used for initialization or the input stream from which the model is loaded. If this parameter is an input stream than no other parameters are required.
 * @example
 * // import analytics module
 * var analytics = require('qminer').analytics;
 * // create the Logistic Regression model
 * var logreg = new analytics.LogReg({ lambda: 2 });
 */
//# exports.LogReg = function (opts) { return Object.create(require('qminer').analytics.LogReg.prototype); }
class TNodeJsLogReg : public node::ObjectWrap {
	friend class TNodeJsUtil;
public:
	static void Init(v8::Handle<v8::Object> exports);
	static const TStr GetClassId() { return "LogReg"; }

private:
	TClassification::TLogReg LogReg;

	TNodeJsLogReg(const TClassification::TLogReg& _LogReg): LogReg(_LogReg) {}

	static TNodeJsLogReg* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

public:

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
	//# exports.LogReg.prototype.getParams = function () { return { lambda: 1.0, intercept: false } };
	JsDeclareFunction(getParams);

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
	//# exports.LogReg.prototype.setParams = function () { return Object.create(require('qminer').analytics.LogReg.prototype); }
	JsDeclareFunction(setParams);

	/**
	 * Fits a column matrix of feature vectors X onto the response variable y.
	 * @param {module:la.Matrix} X - the column matrix which stores the feature vectors.
	 * @param {module:la.Vector} y - the response variable.
	 * @param {number} [eps] - the epsilon used for convergence.
	 * @returns {module:analytics.LogReg} Self.
	 * @example
	 * // import modules
	 * var analytics = require('qminer').analytics;
	 * var la = require('qminer').la;
	 * // create the logistic regression model
	 * var logreg = new analytics.LogReg();
	 * // create the input matrix and vector for fitting the model
	 * var mat = new la.Matrix([[1, 0, -1, 0], [0, 1, 0, -1]]);
	 * var vec = new la.Vector([1, 0, -1, -2]);
	 * // if openblas is used, fit the model
	 * if (require('qminer').flags.blas) {
	 *     logreg.fit(mat, vec);
	 * }
	 */
	//# exports.LogReg.prototype.fit = function (X, y, eps) { return Object.create(require('qminer').analytics.LogReg.prototype); }
	JsDeclareFunction(fit);

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
	 * // if openblas is used
	 * if (require('qminer').flags.blas) {
	 *     // fit the model
	 *     logreg.fit(mat, vec);
	 *     // create the vector for the prediction
	 *     var test = new la.Vector([1, 1]);
	 *     // get the prediction
	 *     var prediction = logreg.predict(test);
	 * };
	 */
	//# exports.LogReg.prototype.predict = function (x) { return 0.0; } 
	JsDeclareFunction(predict);

	/**
	 * Gives the weights of the model.
	 */
	//# exports.LogReg.prototype.weights = Object.create(require('qminer').la.vector.prototype);
	JsDeclareProperty(weights);

	/**
	 * Saves the model into the output stream.
	 * @param {module:fs.FOut} fout - the output stream.
	 * @returns {module:fs.FOut} The output stream fout.
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
	//# exports.LogReg.prototype.save = function (fout) { return Object.create(require('qminer').fs.FOut.prototype); }
	JsDeclareFunction(save);
};

/////////////////////////////////////////////
// Proportional Hazards Model

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
 * @property {module:analytics~hazardModelParam|module:fs.FIn} [opts] - The options used for initialization or the input stream from which the model is loaded. If this parameter is an input stream than no other parameters are required.
 * @example
 * // import analytics module
 * var analytics = require('qminer').analytics;
 * // create a Proportional Hazard model
 * var hazard = new analytics.PropHazards();
 */
//# exports.PropHazards = function (opts) { return Object.create(require('qminer').analytics.PropHazards.prototype); }

class TNodeJsPropHaz : public node::ObjectWrap {
	friend class TNodeJsUtil;
public:
	static void Init(v8::Handle<v8::Object> exports);
	static const TStr GetClassId() { return "PropHazards"; }

private:
	TRegression::TPropHazards Model;

	TNodeJsPropHaz(const TRegression::TPropHazards& _Model): Model(_Model) {}

	static TNodeJsPropHaz* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

public:

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
	//# exports.PropHazards.prototype.getParams = function () { return { lambda: 0.0 }; }
	JsDeclareFunction(getParams);

	/**
	* Sets the parameters of the model.
	* @param {module:analytics~hazardModelParam} params - The parameters given to the model.
	* @returns {module:analytics.PropHazards} Self.
	* @example 
	* // import analytics module
	* var analytics = require('qminer').analytics;
	* // create a Proportional Hazard model
	* var hazard = new analytics.PropHazards({ lambda: 5 });
	* // set the parameters of the model
	* hazard.setParams({ lambda: 10 });
	*/
	//# exports.PropHazards.prototype.setParams = function (params) { return Object.create(require('qminer').analytics.PropHazards.prototype); }
	JsDeclareFunction(setParams);

	/**
	 * Fits a column matrix of feature vectors X onto the response variable y.
	 *
	 * @param {module:la.Matrix} X - The column matrix which stores the feature vectors.
	 * @param {module:la.Vector} y - The response variable.
	 * @param {number} [eps] - The epsilon used for convergence.
	 * @returns {module:analytics.PropHazards} Self.
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
	//# exports.PropHazards.prototype.fit = function(X, y, eps) { return Object.create(require('qminer').analytics.PropHazards.prototype); }
	JsDeclareFunction(fit);

	/**
	 * Returns the expected response for the provided feature vector.
	 *
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
	 * // if openblas used
	 * if (require('qminer').flags.blas) {
	 *     // fit the model
	 *     hazards.fit(mat, vec);       
	 *     // create a vector for the prediction
	 *      var test = new la.Vector([1, 2]);
	 *     // predict the value
	 *     var prediction = hazards.predict(test);
	 * };
	 */
	//# exports.PropHazards.prototype.predict = function(x) { return 0.0; }
	JsDeclareFunction(predict);

	/**
	 * The models weights.
	 */
	//# exports.PropHazards.prototype.weights = Object.create(require('qminer').la.Vector.prototype);
	JsDeclareProperty(weights);

	/**
	 * Saves the model into the output stream.
	 * @param {module:fs.FOut} sout - The output stream.
	 * @returns {module:fs.FOut} The output stream sout.
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
	//# exports.PropHazards.prototype.save = function(sout) { return Object.create(require('qminer').fs.FOut.prototype); }
	JsDeclareFunction(save);
};


////////////////////////////////////////////////////////
// Hierarchical Markov Chain model
//!
//! **Constructor:**
//!
//!- `hmc = new analytics.HMC(params)` -- Creates a new model using `params` JSON. TODO param description.
//!- `hmc = new analytics.HMC(fin)` -- Loads the model from input stream `fin`.
class TNodeJsStreamStory : public node::ObjectWrap, public TMc::TStreamStory::TCallback {
	friend class TNodeJsUtil;
public:
	static void Init(v8::Handle<v8::Object> exports);
	static const TStr GetClassId() { return "_StreamStory"; }

private:
	const static double DEFAULT_DELTA_TM;

	TMc::TStreamStory* StreamStory;

	v8::Persistent<v8::Function> StateChangedCallback;
	v8::Persistent<v8::Function> AnomalyCallback;
	v8::Persistent<v8::Function> OutlierCallback;
	v8::Persistent<v8::Function> PredictionCallback;

	TNodeJsStreamStory(TMc::TStreamStory* McModel);
	TNodeJsStreamStory(PSIn& SIn);

	~TNodeJsStreamStory();

	static TNodeJsStreamStory* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

private:
	class TFitTask: public TNodeTask {
	private:
		TNodeJsStreamStory* JsStreamStory;
		TNodeJsFltVV* JsObservFtrs;
		TNodeJsFltVV* JsControlFtrs;
		TNodeJsFltV* JsRecTmV;
		TNodeJsBoolV* JsBatchEndJsV;

	public:
		TFitTask(const v8::FunctionCallbackInfo<v8::Value>& Args);

		v8::Handle<v8::Function> GetCallback(const v8::FunctionCallbackInfo<v8::Value>& Args);
		void Run();
	};

public:

	JsDeclareSyncAsync(fit,fitAsync,TFitTask);

	//!- `hmc.update(ftrVec, recTm)` TODO write documentation
	JsDeclareFunction(update);

	/**
	 * Returns the probability distribution over the future states given that the current state is the one in
	 * the parameter.
	 *
	 * @param {Number} level - the level on which we want the future states
	 * @param {Number} startState - the ID of the current state (the state we are starting from)
	 * @param {Number} [time] - optional parameter, if not specified the distribution of the next state will be returned
	 * @returns {Array} - the probability distribution
	 */
	JsDeclareFunction(futureStates);

	/**
	 * Returns the probability distribution over the past states given that the current state is the one in
	 * the parameter.
	 *
	 * @param {Number} level - the level on which we want the past states
	 * @param {Number} startState - the ID of the current state (the state we are starting from)
	 * @param {Number} [time] - optional parameter, if not specified the distribution of the previous state will be returned
	 * @returns {Array} - the probability distribution
	 */
	JsDeclareFunction(pastStates);

	/**
	 * Returns the probability distribution of past and future states over time.
	 *
	 * @param {Number} stateId - ID if the starting state
	 * @param {Number} height - the hieght
	 * @param {Number} time - the time at which we want the probabilities
	 * @returns {Array} - array of state ids and their probabilities
	 */
	JsDeclareFunction(probsAtTime);

	/**
	 * Returns information about previous states.
	 *
	 * @param {Number} level - the level on which we want the past states
	 * @retuns {Array} - information about the past states
	 */
	JsDeclareFunction(histStates);

	/**
	 * Returns an object representation of this model.
	 *
	 * @returns {Object}
	 */
	JsDeclareFunction(toJSON);

	/**
	 * Returns the underlying transition model at the lowest level. (for CTMC the matrix of intensities)
	 *
	 * @returns {Array} - the transition model
	 */
	JsDeclareFunction(getTransitionModel);

	/**
	 * Returns the current state throughout the hierarchy. If the level is specified it
	 * will return the current state only on that level.
	 *
	 * @param {Number} [level] - optional level parameter
	 * @returns {Array|Number} - if the level is specified it returns info about the current state on that level, otherwise it return info about the current state on each level on the hierarchy
	 */
	JsDeclareFunction(currState);

	/**
	 * Returns the centroid of the specified state containing only the observation parameters.
	 *
	 * @param {Number} stateId - the ID of the state
	 * @param {Boolean} [observations=true] - indicates wether to output observation or control coordinates
	 * @returns {Array} - the coordinates of the state
	 */
	JsDeclareFunction(fullCoords);

	/**
	 * Returns a histogram of the specified feature in the specified state.
	 *
	 * @param {Number} stateId - the ID of the state
	 * @param {Number} ftrId - the ID of the feature
	 * @returns {Array} - the histogram
	 */
	JsDeclareFunction(histogram);

	JsDeclareFunction(transitionHistogram);

	/**
	 * Returns the lower and upper bound of the feature.
	 *
	 * @param {Integer} ftrId - id of the feature
	 */
	JsDeclareFunction(getFtrBounds);

	/**
	 * Returns an array of IDs of all the states on the specified height.
	 *
	 * @param {Number} height - the height
	 * @returns {Array} - the array of IDs
	 */
	JsDeclareFunction(stateIds);

	/**
	 * Returns the weights of features in this state.
	 *
	 * @param {Number} stateId - The Id of the state.
	 * @returns {Array} - An array of weights.
	 */
	JsDeclareFunction(getStateWgtV);

	/**
	 * Returns a JSON representation of a decision tree, which classifies
	 * this state against other states
	 *
	 * @param {Number} stateId
	 * @returns {Object}
	 */
	JsDeclareFunction(getClassifyTree);

	JsDeclareFunction(explainState);

	/**
	 * Sets a callback function which is fired when the model changes states. An array of current states
	 * throughout the hierarchy is passed to the callback.
	 *
	 * @param {function} callback - the funciton which is called
	 */
	JsDeclareFunction(onStateChanged);

	/**
	 * Sets a callback function which is fired when the model detects an anomaly. A string description is
	 * passed to the callback.
	 *
	 * @param {function} callback - the funciton which is called
	 */
	JsDeclareFunction(onAnomaly);

	/**
	 * Sets a callback function which is fired when the model detects an outlier. A string description is
	 * passed to the callback.
	 *
	 * @param {function} callback - the funciton which is called
	 */
	JsDeclareFunction(onOutlier);

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
	JsDeclareFunction(onPrediction);

	/**
	 * Rebuilds its hierarchy.
	 */
	JsDeclareFunction(rebuildHierarchy);

	/**
	 * Rebuilds the histograms using the instances stored in the columns of X.
	 *
	 * @param {Matrix} obsMat - the column matrix containing observation data instances
	 * @param {Matrix} controlMat - the column matrix containing control data instances
	 */
	JsDeclareFunction(rebuildHistograms);

	/**
	 * Returns the name of a state.
	 *
	 * @param {Number} stateId - ID of the state
	 * @returns {String} - the name of the state
	 */
	JsDeclareFunction(getStateName);

	/**
	 * Sets the name of the state.
	 *
	 * @param {Number} stateId - ID of the state
	 * @param {String} name - name of the state
	 */
	JsDeclareFunction(setStateName);

	/**
	 * Sets the name of the state.
	 *
	 * @param {Number} stateId - ID of the state
	 */
	JsDeclareFunction(clearStateName);

	/**
	 * Returns true if the state is a target on the specified height.
	 *
	 * @param {Number} stateId - Id of the state
	 * @param {Number} height - the height
	 * @returns {Boolean}
	 */
	JsDeclareFunction(isTarget);

	/**
	 * Sets whether the specified state is a target state or not.
	 *
	 * @param {Number} stateId - ID of the state
	 * @param {Number} height - the height on which the state is a target
	 * @param {Boolean} isTarget - set target on/off
	 */
	JsDeclareFunction(setTarget);

	/**
	 * Returns true if the state defined by the ID is at the bottom of the hierarchy.
	 *
	 * @param {Number} stateId - ID of the state
	 */
	JsDeclareFunction(isLeaf);

	/**
	 * Returns the time unit used by this model.
	 *
	 * @returns {String} timeUnit
	 */
	JsDeclareFunction(getTimeUnit);

	/**
	 * Sets the factor of the specified control:
	 *
	 * @param {Object} params - the parameters
	 * @property {Number} [params.stateId] - id of the state, if not present, all the states will be set
	 * @property {Number} params.ftrId - the index of the control feature
	 * @property {Number} params.val - the value of the featuere
	 */
	JsDeclareFunction(setControlVal);

	JsDeclareFunction(resetControlVal);

	/**
	 * Returns true is any of the control parameters have been set in any of the states.
	 *
	 * @returns {Boolean}
	 */
	JsDeclareFunction(isAnyControlFtrSet);

	// parameters
	//!- `hmc = hmc.getParams(params)` -- sets one or more parameters given
	//!- in the input argument `params` returns this
	JsDeclareFunction(setParams);

	JsDeclareFunction(getParam);

	/**
	 * Saves the model to the output stream.
	 *
	 * @param {FOut} fout - the output stream
	 */
	JsDeclareFunction(save);

	// TMcCallback - callbacks
	void OnStateChanged(const TIntFltPrV& StateIdHeightV);
	void OnAnomaly(const TStr& AnomalyDesc);
	void OnOutlier(const TFltV& FtrV);
	void OnPrediction(const uint64& RecTm, const int& CurrStateId, const int& TargetStateId,
			const double& Prob, const TFltV& ProbV, const TFltV& TmV);

private:
	void SetParams(const PJsonVal& ParamVal);
	void InitCallbacks();

	static void WrapHistogram(const v8::FunctionCallbackInfo<v8::Value>& Args,
			const TFltV& BinStartV, const TFltV& ProbV);
	static uint64 GetTmUnit(const TStr& TmUnitStr);
	static TClustering::PDnsKMeans GetClust(const PJsonVal& ParamJson, const TRnd& Rnd);
};

///////////////////////////////
// QMiner-JavaScript-Neural-Networks

/**
* @typedef {Object} nnetParams
* @property {module:la.IntVector} [layout] - The integer vector with the corresponding values of the number of neutrons. Default is the integer vector [1, 2 ,1].
* @property {number} [learnRate = 0.1] - The learning rate.
* @property {number} [momentum = 0.5] - The momentum of optimization.
* @property {string} [tFuncHidden = 'tanHyper'] - The function.
* @property {string} [tFuncOut = 'tanHyper'] - The function.
*/

/**
* Neural Network Model
* @classdesc Holds online/offline neural network model.
* @class
* @param {module:analytics~nnetParams|module:fs.FIn} [params] - The parameters for the construction of the model.
*/
//# exports.NNet = function (params) { return Object.create(require('qminer').analytics.NNet.prototype); }
class TNodeJsNNet : public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	TSignalProc::PNNet Model;

	TNodeJsNNet(const PJsonVal& ParamVal);
	TNodeJsNNet(TSIn& SIn);
	static TNodeJsNNet* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

public:
	static void Init(v8::Handle<v8::Object> exports);
	static const TStr GetClassId() { return "NNet"; }

	/**
	* Get the parameters of the model.
	* @returns {module:analytics~nnetParams} The constructor parameters.
	* @example
	* // import analytics module
	* var analytics = require('qminer').analytics;
	* // create a Neural Networks model
	* var nnet = new analytics.NNet();
	* // get the parameters
	* var params = nnet.getParams();
	*/
	//# exports.NNet.prototype.getParams = function () { return { layout: Object.create(require('qminer').la.IntVector.prototype), learnRate: 0.0, momentum: 0.0, tFuncHidden: "", TFuncOut: "" }; }
	JsDeclareFunction(getParams);

	/**
	* Sets the parameters of the model.
	* @params {module:analytics~nnetParams} params - The given parameters.
	* @returns {module:analytics.NNet} Self.
	* @example
	* // import analytics module
	* var analytics = require('qminer').analytics;
	* // create a Neural Networks model
	* var nnet = new analytics.NNet();
	* // set the parameters
	* nnet.setParams({ learnRate: 1, momentum: 10, layout: [1, 4, 3] });
	*/
	//# exports.NNet.prototype.setParams = function (params) { return Object.create(require('qminer').analytics.NNet.prototype); }
	JsDeclareFunction(setParams);

	/**
	* Fits the model.
	* @param {(module:la.Vector|module:la.Matrix)} input1 - The input vector or matrix.
	* @param {(module:la.Vector|module:la.Matrix)} input2 - The input vector or matrix.
	* <br> If input1 and input2 are both {@link module:la.Vector}, then the fitting is in online mode.
	* <br> If input1 and input2 are both {@link module:la.Matrix}, then the fitting is in batch mode.
	* @returns {module:analytics.NNet} Self.
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
	//# exports.NNet.prototype.fit = function (input1, input2) { return Object.create(require('qminer').analytics.NNet.prototype); }
	JsDeclareFunction(fit);
	
	/**
	* Sends the vector through the model and get the prediction.
	* @param {module:la.Vector} vec - The sent vector.
	* @returns {number} The prediction of the vector vec.
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
	* // predict the value
	* var prediction = nnet.predict(test);
	*/
	//# exports.NNet.prototype.predict = function (vec) { return 0.0; }
	JsDeclareFunction(predict);

	/**
	* Saves the model.
	* @param {module:fs.FOut} fout - The output stream.
	* @returns {module:fs.FOut} The output stream fout.
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
	//# exports.NNet.prototype.save = function (fout) { return Object.create(require('qminer').fs.FOut.prototype); } 
	JsDeclareFunction(save);
 private:
	TSignalProc::TTFunc ExtractFuncFromString(const TStr& FuncString);
};

/////////////////////////////////////////////
// QMiner-JavaScript-Tokenizer

/**
* @typedef {Object} tokenizerParam
* @property {string} type - The type of the tokenizer. The different types are: 
*<br>"simple" -
*<br>"html" -
*<br>"unicode" -
*/

/**
 * Tokenizer
 * @class 
 * @classdesc Breaks text into tokens (i.e. words).
 * @param {module:analytics.tokenizerParam} param - The constructor parameters.
 * @example
 * // import analytics module
 * var analytics = require('qminer').analytics;
 * // construct model
 * var tokenizer = new analytics.Tokenizer({ type: "simple" })
 */
//# exports.Tokenizer = function (param) { return Object.create(require("qminer").analytics.Tokenizer.prototype); }
class TNodeJsTokenizer : public node::ObjectWrap {
	friend class TNodeJsUtil;
public:
	/// Tokenizer Model
	PTokenizer Tokenizer;
private:
	TNodeJsTokenizer(const PTokenizer& _Tokenizer): 
		Tokenizer(_Tokenizer) { }
public:
	static void Init(v8::Handle<v8::Object> exports);
	static const TStr GetClassId() { return "Tokenizer"; }
	
	static TNodeJsTokenizer* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

	// Functions:
	/**
	* This function tokenizes given strings and returns it as an array of strings.
	* @param {String} str - String of text you want to tokenize.
	* @returns {Array.<String>} Returns array of strings. The number of strings in this array is equal to number of words in input string parameter.
	* Only keeps words, skips all punctuation.
	* Tokenizing contractions (i.e. don't) depends on which type you use. Type 'html' breaks contractions into 2 tokens.
	* @example
	* // import modules
	* var analytics = require('qminer').analytics;
	* var la = require('qminer').la;
	* // construct model
	* var tokenizer = new analytics.Tokenizer({ type: "simple" });
	* // string you wish to tokenize
	* var string = "What a beautiful day!";
	* // tokenize string using getTokens
	* var tokens = tokenizer.getTokens(string);
	* // output:
	* tokens = ["What", "a", "beautiful", "day"];
	*/
	//# exports.Tokenizer.prototype.getTokens = function (str) { return [""]; }
	JsDeclareFunction(getTokens);

	/**
	* This function breaks text into sentences and returns them as an array of strings.
	* @param {String} str - String of text you want to break into sentences.
	* @returns {Array.<String>} Returns array of strings. The number of strings in this array is equal to number of sentences in input string parameter.
	* How function breaks sentences depends on where you use a full-stop, exclamation mark, question mark or the new line command.
	* Careful: the space between the lines is not ignored. 
	* With all 3 types this function returns sentences as they are.
	* @example
	* // import modules
	* var analytics = require('qminer').analytics;
	* var la = require('qminer').la;
	* // construct model
	* var tokenizer = new analytics.Tokenizer({ type: "simple" });
	* // string you wish to tokenize
	* var string = "C++? Alright. Let's do this!";
	* // tokenize text using getSentences
	* var tokens = tokenizer.getSentences(string);
	* // output:
	* tokens = ["C++", " Alright", " Let's do this"];
	*/
	//# exports.Tokenizer.prototype.getSentences = function (str) { return [""]; }
	JsDeclareFunction(getSentences);
	
	/**
	* This function breaks text into paragraphs and returns them as an array of strings.
	* @param {String} str - String of text you want to break into paragraphs.
	* @returns {Array.<String>} Returns array of strings. The number of strings in this array is equal to number of paragraphs in input string parameter.
	* When function detects commands '\n', '\r' or '\t' it breaks text as new paragraph.
	* With all 3 types this function returns paragraphs as they are.
	* @example
	* // import modules
	* var analytics = require('qminer').analytics;
	* var la = require('qminer').la;
	* // construct model
	* var tokenizer = new analytics.Tokenizer({ type: "simple" });
	* // string you wish to tokenize
	* var string = "Yes!\t No?\n Maybe...";
	* // tokenize text using getParagraphs
	* var tokens = tokenizer.getParagraphs(string);
	* // output:
	* tokens = ["Yes", " No", " Maybe"];
	*/
	//# exports.Tokenizer.prototype.getParagraphs = function (str) { return [""]; }
	JsDeclareFunction(getParagraphs);
	
};

/**
* @typedef {Object} MDSParam
* @property {number} [maxSecs=500] - The maximum time period to compute MDS of a matrix.
* @property {number} [maxStep=5000] - The maximum number of iterations.
* @property {number} [minDiff=1e-4] - The minimum difference criteria in MDS.
* @property {string} [distType="Euclid"] - The type of distance used. Available types: "Euclid", "Cos", "SqrtCos".
*/

/**
* @class
* @classdesc Multidimensional scaling
* @param {(module:analytics~MDSParam | module:fs.FIn)} [params] - The parameters for the construction.
* @example
* // import analytics module
* var analytics = require('qminer').analytics;
* // construct a MDS instance
* var mds = new analytics.MDS({ maxStep: 300, distType: 'Cos' });
*/
//# exports.MDS = function (params) { return Object.create(require('qminer').analytics.MDS.prototype); }
class TNodeJsMDS : public node::ObjectWrap {
	friend class TNodeJsUtil;
public:
	static void Init(v8::Handle<v8::Object> exports);
	static const TStr GetClassId() { return "MDS"; }

private:
	// parameters
	int MxStep, MxSecs;
	double MnDiff;
	TVizDistType DistType;

	TNodeJsMDS(const PJsonVal& ParamVal);
	TNodeJsMDS(TSIn& SIn);

	static TNodeJsMDS* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

public:
	/**
	* Get the parameters.
	* @returns {module:analytics~MDSParam} The json object containing the parameters of the instance.
	* @example
	* // import analytics module
	* var analytics = require('qminer').analytics;
	* // create a MDS instance
	* var mds = new analytics.MDS();
	* // get the (default) parameters of the instance
	* // returns { maxStep: 5000, maxSecs: 300, minDiff: 1e-4, distType: "Euclid" }
	* var params = mds.getParams();
	*/
	//# exports.MDS.prototype.getParams = function () { return { maxStep: 0, maxSecs: 0, minDiff: 0, distType: "" }; }
	JsDeclareFunction(getParams);

	/**
	* Set the parameters.
	* @param {module:analytics~MDSParam} params - The json object containing the parameters for the instance.
	* @example
	* // import analytics module
	* var analytics = require('qminer').analytics;
	* // create a MDS instance
	* var mds = new analytics.MDS();
	* // get the (default) parameters of the instance
	* // returns { maxStep: 5000, maxSecs: 300, minDiff: 1e-4, distType: "Euclid" }
	* var params = mds.getParams();
	*/
	//# exports.MDS.prototype.setParams = function (params) { return { maxStep: 0, maxSecs: 0, minDiff: 0, distType: "" }; }
	JsDeclareFunction(setParams);

	/**
	* Get the MDS of the given matrix.
	* @param {(module:la.Matrix | module:la.SparseMatrix)} mat - The multidimensional matrix.
	* @returns {module:la.Matrix} The matrix of dimensions mat.cols x 2, where the i-th row of the matrix is the 2d representation 
	* of the i-th column of mat.
	* @example
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
	//# exports.MDS.prototype.fitTransform = function (mat) { return Object.create(require('qminer').la.Matrix.prototype); }
	JsDeclareFunction(fitTransform);

	/**
	* Save the MDS.
	* @param {module:fs.FOut} fout - The output stream.
	* @returns {module:fs.FOut} The output stram fout.
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
	//# exports.MDS.prototype.save = function (fout) { return Object.create(require('qminer').fs.FOut.prototype); }
	JsDeclareFunction(save);

private:
	void UpdateParams(const PJsonVal& ParamVal);
	PJsonVal GetParams() const;
	void Save(TSOut& SOut) const;
};


#endif /* ANALYTICS_H_ */


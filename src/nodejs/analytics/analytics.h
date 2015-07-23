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
	JsDeclareFunction(New);
	//- `params = svmModel.getParams()` -- returns the parameters of this model as a Javascript object
	JsDeclareFunction(getParams);
	//- `svmModel = svmModel.getParams(params)` -- sets one or more parameters given in the input argument `params` returns this
	JsDeclareFunction(setParams);
    //- `vec = svmModel.weights` -- weights of the SVM linear model as a full vector `vec`
	JsDeclareProperty(weights);
    //- `fout = svmModel.save(fout)` -- saves model to output stream `fout`. Returns `fout`.
	JsDeclareFunction(save);
	//- `num = svmModel.decision_function(vec)` -- sends vector `vec` through the model and returns the distance to the decision boundery as a real number `num`
    //- `num = svmModel.decision_function(spVec)` -- sends sparse vector `spVec` through the model and returns the distance to the decision boundery as a real number `num`
	JsDeclareFunction(decision_function);
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
* SVC.save('svc.bin');
* // Set up a fake test vector
* var test = new la.Vector([1.1, -0.5]);
* // Predict the target value
* var prediction = SVC.predict(test);
*/
//# exports.SVC = function(arg) {};

class TNodeJsSVC : public TNodeJsSvmModel {
	static v8::Persistent <v8::Function> constructor;
public:
	static void Init(v8::Handle<v8::Object> exports);

	/**
	* returns the svc parameters	
	* @returns {module:analytics~svcParam} Parameters of the classifier model.
	*/
	//# exports.SVC.prototype.getParams = function() {};

	/**
	* sets the svc parameters
	* @param {module:analytics~svcParam} param - Classifier training parameters.
	*/
	//# exports.SVC.prototype.setParams = function(param) {};

	/**	
	* @property {module:la.Vector} weights - Vector of coefficients of the linear model
	*/
	//# exports.SVC.prototype.weights = undefined;

	/**
	* saves model to output file stream 
	* @param {module:fs.FOut} fout - Output stream.
	* @returns {module:fs.FOut} Output stream
	*/
	//# exports.SVC.prototype.save = function(fout) {}

    /**
     * sends vector through the model and returns the distance to the decision boundery
     * @param {module:la.Vector | module:la.SparseVector} vec - Input vector
     * @returns {number} Prediction real number. Sign of the number corresponds to the class and the magnitude corresponds to the distance from the margin (certainty).
     */
    //# exports.SVC.prototype.decision_function = function(vec) {}
    
	/**
	* sends vector through the model and returns the prediction as a real number
	* @param {module:la.Vector | module:la.SparseVector} vec - Input vector
	* @returns {number} Prediction, 1 for positive class and -1 for negative.
	*/
	//# exports.SVC.prototype.predict = function(vec) {}
	
	/**
	* fits an SVM classification model, given column examples in a matrix and vector of targets
	* @param {module:la.Matrix | module:la.SparseMatrix} X - Input feature matrix where columns correspond to feature vectors
	* @param {module:la.Vector} y - Input vector of targets, one for each column of X
	*/
	//# exports.SVC.prototype.fit = function(X, y) {}
	JsDeclareFunction(fit);
};

///////////////////////////////
// QMiner-JavaScript-Support-Vector-Regression

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
* SVR.save('svr.bin');
* // Set up a fake test vector
* var test = new la.Vector([1.1, -0.8]);
* // Predict the target value
* var prediction = SVR.predict(test);
*/
//# exports.SVR = function(arg) {};

class TNodeJsSVR : public TNodeJsSvmModel {
	static v8::Persistent <v8::Function> constructor;
public:
	static void Init(v8::Handle<v8::Object> exports);
    
	/**
	* returns the svr parameters
	* @returns {module:analytics~svrParam} Parameters of the regression model.
	*/
	//# exports.SVR.prototype.getParams = function() {};

	/**
	* sets the svr parameters
	* @param {module:analytics~svrParam} param - Regression training parameters.
	*/
	//# exports.SVR.prototype.setParams = function(param) {};

	/**
	* @property {module:la.Vector} weights - Vector of coefficients of the linear model
	*/
	//# exports.SVR.prototype.weights = undefined;

	/**
	* saves model to output file stream
	* @param {module:fs.FOut} fout - Output stream.
	* @returns {module:fs.FOut} Output stream
	*/
	//# exports.SVR.prototype.save = function(fout) {}

    /**
     * sends vector through the model and returns the prediction as a real number
     * @param {module:la.Vector | module:la.SparseVector} vec - Input vector
     * @returns {number} Prediction real number.
     */
    //# exports.SVR.prototype.decision_function = function(vec) {}

	/**
	* sends vector through the model and returns the prediction as a real number
	* @param {module:la.Vector | module:la.SparseVector} vec - Input vector
	* @returns {number} Prediction real number.
	*/
	//# exports.SVR.prototype.predict = function(vec) {}

	/**
	* fits an SVM regression model, given column examples in a matrix and vector of targets
	* @param {module:la.Matrix | module:la.SparseMatrix} X - Input feature matrix where columns correspond to feature vectors
	* @param {module:la.Vector} y - Input vector of targets, one for each column of X
	*/
	//# exports.SVR.prototype.fit = function(X, y) {}
	JsDeclareFunction(fit);	
};

/////////////////////////////////////////////
// Ridge Regression
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
     * Fits a column matrix of feature vectors X onto the response variable y.
     *
     * @param {module:la.Matrix} X - Column matrix which stores the feature vectors.
     * @param {module:la.Vector} y - Response variable.
     * @returns {module:analytics.RidgeReg} Self
     */
    //# exports.RidgeReg.prototype.fit = function(X,y) {}
    JsDeclareFunction(fit);

    /**
     * Returns the expected response for the provided feature vector.
     *
     * @param {module:la.Vector} x - Feature vector
     * @returns {number} Predicted response
     */
    //# exports.RidgeReg.prototype.decision_function = function(x) {}
    /**
     * Returns the expected response for the provided feature vector.
     *
     * @param {module:la.Vector} x - Feature vector
     * @returns {number} Predicted response
     */
    //# exports.RidgeReg.prototype.predict = function(x) {}
    JsDeclareFunction(predict);
    
    /**
     * @property {module:la.Vector} weights - Vector of coefficients for linear regression
     */
    //# exports.RidgeReg.prototype.weights = undefined;
    JsDeclareProperty(weights);
    
    /**
     * Saves the model into the output stream.
     *
     * @param {module:fs.FOut} fout - Output stream
     */
    //# exports.RidgeReg.prototype.save = function(fout) {};
    JsDeclareFunction(save);
};

/////////////////////////////////////////////
// Sigomid
/**
 * Sigmoid funnction (y = 1/[1 + exp[-Ax+B]]) fited on decision function to mimic
 *
 * @class
 * @param {(|module:fs.FIn)} [arg] - Loads a model from input stream, or creates a new model.
 * @example
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
     * Fits a column matrix of feature vectors X onto the response variable y.
     *
     * @param {module:la.Vector} x - Predicted values (e.g., using analytics.SVR)
     * @param {module:la.Vector} y - Actual binary labels: 1 or -1.
     * @returns {module:analytics.Sigmoid} Self
     */
    //# exports.Sigmoid.prototype.fit = function(X,y) {}
    JsDeclareFunction(fit);
    
    /**
     * Returns the expected response for the provided feature vector.
     *
     * @param {(|module:la.Vector)} x - Prediction score (or vector of them).
     * @returns {(|module:la.Vector)} Normalized prediction score (or vector of them).
     */
    //# exports.Sigmoid.prototype.decision_function = function(x) {}
    /**
     * Returns the expected response for the provided feature vector.
     *
     * @param {(|module:la.Vector)} x - Prediction score (or vector of them).
     * @returns {(|module:la.Vector)} Normalized prediction score (or vector of them).
     */
    //# exports.Sigmoid.prototype.predict = function(x) {}
    JsDeclareFunction(predict);
    
    /**
     * @property {module:la.Vector} weights - Vector with elements A and B that define the sigmoid function.
     */
    //# exports.Sigmoid.prototype.weights = undefined;
    JsDeclareProperty(weights);
    
    /**
     * Saves the model into the output stream.
     *
     * @param {module:fs.FOut} fout - Output stream
     */
    //# exports.Sigmoid.prototype.save = function(fout) {};
    JsDeclareFunction(save);
};

///////////////////////////////
////// code below not yet ported or verified for scikit
///////////////////////////////

///////////////////////////////
// QMiner-JavaScript-Recursive-Linear-Regression
//!
//! ### Recursive Linear Regression model
//!
//! Holds online regression model.
class TNodeJsRecLinReg : public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	TSignalProc::PRecLinReg Model;
	TNodeJsRecLinReg(const TSignalProc::PRecLinReg& Model);
public:
	static void Init(v8::Handle<v8::Object> exports);
	static const TStr GetClassId() { return "RecLinReg"; }
private:
	//!
	//! **Constructor:**
	//!
	//!- `recLinRegModel = new analytics.RecLinReg(fin)` -- constructs a recursive linear regression model by loading it from input stream `fin`
	//!- `recLinRegModel = new analytics.RecLinReg(recLinRegParameters)` -- constructs a recursive linear regression using a JSON parameter object `recLinRegParameters, whose properties are `recLinRegParameters.dim` (dimensionality of feature space, e.g.
	//!     `ftrSpace.dim`), `recLinRegParameters.forgetFact` (forgetting factor, default is 1.0) and `recLinRegParameters.regFact` 
	//!     (regularization parameter to avoid over-fitting, default is 1.0).)
	static TNodeJsRecLinReg* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);
	//!
	//! **Functions and properties:**
	//!
    //!- `recLinRegModel = recLinRegModel.fit(vec, num)` -- updates the model using full vector `vec` and target number `num`as training data. Returns self.
	JsDeclareFunction(fit);
    //!- `num = recLinRegModel.predict(vec)` -- sends vector `vec` through the
    //!     model and returns the prediction as a real number `num`
	JsDeclareFunction(predict);

	//!- `params = svmModel.getParams()` -- returns the parameters of this model as
	//!- a Javascript object
	JsDeclareFunction(getParams);

    //!- `vec = recLinRegModel.weights` -- weights of the linear model as a full vector `vec`
	JsDeclareProperty(weights);
    //!- `num = recLinRegModel.dim` -- dimensionality of the feature space on which this model works
	JsDeclareProperty(dim);
	//!- `fout = recLinRegModel.save(fout)` -- saves model to output stream `fout`. Returns `fout`.
	JsDeclareFunction(save);

private:
	PJsonVal GetParams() const;
};


/**
 * Logistic regression model. Uses Newtons method to compute the weights.
 *
 * @constructor
 * @property {Object|FIn} [opts] - The options used for initialization or the input stream from which the model is loaded. If this parameter is an input stream than no other parameters are required.
 * @property {Number} [opts.lambda = 1] - the regularization parameter
 * @property {Boolean} [opts.intercept = false] - indicates wether to automatically include the intercept
 */
class TNodeJsLogReg : public node::ObjectWrap {
	friend class TNodeJsUtil;
public:
	static void Init(v8::Handle<v8::Object> exports);
	static const TStr GetClassId() { return "LogReg"; }

private:
	TRegression::TLogReg LogReg;

	TNodeJsLogReg(const TRegression::TLogReg& _LogReg): LogReg(_LogReg) {}

	static TNodeJsLogReg* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

public:
	/**
	 * Fits a column matrix of feature vectors X onto the response variable y.
	 *
	 * @param {Matrix} X - the column matrix which stores the feature vectors.
	 * @param {Vector} y - the response variable.
	 * @param {Number} [eps] - the epsilon used for convergence
	 * @returns {LogReg} - returns itself
	 */
	JsDeclareFunction(fit);

	/**
	 * Returns the expected response for the provided feature vector.
	 *
	 * @param {Vector} x - the feature vector
	 * @returns {Number} - the expected response
	 */
	JsDeclareFunction(predict);

	/**
	 * The models weights.
	 *
	 * @type {Vector}
	 */
	JsDeclareProperty(weights);

	/**
	 * Saves the model into the output stream.
	 *
	 * @param {FOut} sout - the output stream
	 */
	JsDeclareFunction(save);
};

/////////////////////////////////////////////
// Proportional Hazards Model
/**
 * Proportional Hazards model with a constant hazard function.
 *
 * Uses Newtons method to compute the weights.
 *
 * @constructor
 * @property {Object|FIn} [opts] - The options used for initialization or the input stream from which the model is loaded. If this parameter is an input stream than no other parameters are required.
 * @property {Number} [opts.lambda = 0] - the regularization parameter
 */
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
	 * Fits a column matrix of feature vectors X onto the response variable y.
	 *
	 * @param {Matrix} X - the column matrix which stores the feature vectors.
	 * @param {Vector} y - the response variable.
	 * @param {Number} [eps] - the epsilon used for convergence
	 * @returns {ExpReg} - returns itself
	 */
	JsDeclareFunction(fit);

	/**
	 * Returns the expected response for the provided feature vector.
	 *
	 * @param {Vector} x - the feature vector
	 * @returns {Number} - the expected response
	 */
	JsDeclareFunction(predict);

	/**
	 * The models weights.
	 *
	 * @type {Vector}
	 */
	JsDeclareProperty(weights);

	/**
	 * Saves the model into the output stream.
	 *
	 * @param {FOut} sout - the output stream
	 */
	JsDeclareFunction(save);
};


////////////////////////////////////////////////////////
// Hierarchical Markov Chain model
//#
//# **Constructor:**
//#
//#- `hmc = new analytics.HMC(params)` -- Creates a new model using `params` JSON. TODO param description.
//#- `hmc = new analytics.HMC(fin)` -- Loads the model from input stream `fin`.
class TNodeJsStreamStory : public node::ObjectWrap, public TMc::TStreamStory::TCallback {
	friend class TNodeJsUtil;
public:
	static void Init(v8::Handle<v8::Object> exports);
	static const TStr GetClassId() { return "HMC"; }

private:
	const static double DEFAULT_DELTA_TM;

	TMc::PStreamStory StreamStory;

	v8::Persistent<v8::Function> StateChangedCallback;
	v8::Persistent<v8::Function> AnomalyCallback;
	v8::Persistent<v8::Function> OutlierCallback;
	v8::Persistent<v8::Function> PredictionCallback;

	TNodeJsStreamStory(const TMc::PStreamStory& McModel);
	TNodeJsStreamStory(PSIn& SIn);

	~TNodeJsStreamStory();

	static TNodeJsStreamStory* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

public:
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
	JsDeclareFunction(fit);
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
	 * @param {Number} level - the level on which we want the distributions
	 * @param {Number} state - the state we are starting from
	 * @param {Number} dt - the time step (lower dt => more distributions will be returned)
	 * @returns {Array} - array of probability distributions over time
	 */
	JsDeclareFunction(probsOverTime);

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
	 * Sets the factor of the specified control:
	 *
	 * @param {Number} ftrIdx - the index of the control feature
	 * @param {Number} factor
	 */
	JsDeclareFunction(setControlFactor);

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
	void OnPrediction(const int& CurrStateId, const int& TargetStateId,
			const double& Prob, const TFltV& ProbV, const TFltV& TmV);

private:
	void SetParams(const PJsonVal& ParamVal);
	void InitCallbacks();
};

///////////////////////////////
// QMiner-JavaScript-Neural-Networks
//!
//! ### Neural Network model
//!
//! Holds online/offline neural network model. This object is result of `analytics.newNNet`.
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

    //!- `NNet = NNet.fit(vec,vec)` -- fits the NNet model in online mode
    //!- `NNet = NNet.fit(mat,mat)` -- fits the NNet model in batch mode
	JsDeclareFunction(fit);
    //!- `vec = NNet.predict(vec)` -- sends vector `vec` through the
    //!     model and returns the prediction as a vector `vec`
	JsDeclareFunction(predict);
	//!- `NNet.setLearnRate(num)` -- Sets the new learn rate for the network
	JsDeclareFunction(setLearnRate);
	//!- `NNet.save(fout)` -- Saves the model into the specified output stream.
	JsDeclareFunction(save);
 private:
	TSignalProc::TTFunc ExtractFuncFromString(const TStr& FuncString);
};

// QMiner-JavaScript-Tokenizer
//!
//! ### Tokenizer
//!
//! Breaks text into tokens (i.e. words).
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

	
	//!
	//! **Constructor:**
	//!
	//!- `tokenizer = new analytics.Tokenizer({ type: <type>, ...})` -- create new tokenizer
	//!     of type `<type>`. Syntax same as when defining index keys in stores or `text` feature 
	//!     extractors.
	static TNodeJsTokenizer* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

	//!
	//! **Functions and properties:**
	//!
	//!- `arr = tokenizer.getTokens(string)` -- tokenizes given strings and returns it as an array of strings.
	JsDeclareFunction(getTokens);
	//!- `arr = tokenizer.getSentences(string)` -- breaks text into sentence and returns them as an array of strings.
	JsDeclareFunction(getSentences);
	//!- `arr = tokenizer.getParagraphs(string)` -- breaks text into paragraphs and returns them as an array of strings.
	JsDeclareFunction(getParagraphs);
};

#endif /* ANALYTICS_H_ */


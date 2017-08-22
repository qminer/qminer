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
#include "qminer_core.h"
#include "../../glib/mine/mine.h"
#include "graphprocess.h"

/**
 * Analytics module.
 * @module analytics
 * @example
 * // import modules
 * var qm = require('qminer');
 * var analytics = qm.analytics;
 * // load dataset, create model, evaluate model
 */
class TNodeJsAnalytics : public node::ObjectWrap {
    friend class TNodeJsUtil;
public:
    static void Init(v8::Handle<v8::Object> exports);

private:
    class TNMFTask : public TNodeTask {
    private:
        TNodeJsFltVV* JsFltVV;
        TNodeJsSpMat* JsSpVV;
        TNodeJsFltVV* U;
        TNodeJsFltVV* V;
        int k;
        int Iter;
        double Tol;
        PNotify Notify;

    public:
        TNMFTask(const v8::FunctionCallbackInfo<v8::Value>& Args);

        v8::Handle<v8::Function> GetCallback(const v8::FunctionCallbackInfo<v8::Value>& Args);
        void Run();
        v8::Local<v8::Value> WrapResult();
    };
public:
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
    //# exports.prototype.nmf = function (mat, k, json) { return { "U": Object.create(require('qminer').la.Matrix.prototype), "V": Object.create(require('qminer').la.Matrix.prototype) }; }
    JsDeclareSyncAsync(nmf, nmfAsync, TNMFTask);
};


///////////////////////////////
// QMiner-JavaScript-Support-Vector-Machine-Model
// Holds SVM classification or regression model.
class TNodeJsSvmModel : public node::ObjectWrap {
    friend class TNodeJsUtil;
    friend class TNodeJsSVC;
    friend class TNodeJsSVR;
public:
    static const TStr GetClassId() { return "SvmModel"; }
    ~TNodeJsSvmModel() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; }
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
//# exports.SVC = function(arg) { return Object.create(require('qminer').analytics.SVC.prototype); };

class TNodeJsSVC : public TNodeJsSvmModel {
public:
    static void Init(v8::Handle<v8::Object> exports);

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
    //# exports.SVC.prototype.getParams = function() { return { algorithm: '', c: 0, j: 0, eps: 0.1, batchSize: 0, maxIterations: 0, maxTime: 0, minDiff: 0, verbose: true } };

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
    //# exports.SVC.prototype.setParams = function(param) { return Object.create(require('qminer').analytics.SVC.prototype); };

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
    //# exports.SVC.prototype.weights = Object.create(require('qminer').la.Vector.prototype);

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
    //# exports.SVC.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); }

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
    //# exports.SVC.prototype.decisionFunction = function(X) { return (X instanceof require('qminer').la.Vector | X instanceof require('qminer').la.SparseVector) ? 0 : Object.create(require('qminer').la.Vector.prototype); }

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
    //# exports.SVC.prototype.predict = function(X) { return (X instanceof require('qminer').la.Vector | X instanceof require('qminer').la.SparseVector) ? 0 : Object.create(require('qminer').la.Vector.prototype); }

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
    //# exports.SVC.prototype.fit = function(X, y) { return Object.create(require('qminer').analytics.SVC.prototype); }
    JsDeclareFunction(fit);
};

///////////////////////////////
// QMiner-JavaScript-Support-Vector-Regression

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
//# exports.SVR = function(arg) { return Object.create(require('qminer').analytics.SVR.prototype); };

class TNodeJsSVR : public TNodeJsSvmModel {
public:
    static void Init(v8::Handle<v8::Object> exports);

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
    //# exports.SVR.prototype.getParams = function() { return { algorithm: '', c: 0, j: 0, eps: 0, batchSize: 0, maxIterations: 0, maxTime: 0, minDiff: 0, verbose: true } };

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
    //# exports.SVR.prototype.setParams = function(param) { return Object.create(require('qminer').analytics.SVR.prototype); };

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
    //# exports.SVR.prototype.weights = Object.create(require('qminer').la.Vector.prototype);

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
    //# exports.SVR.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); }

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
    //# exports.SVR.prototype.decisionFunction = function(X) { return (X instanceof require('qminer').la.Vector | X instanceof require('qminer').la.SparseVector) ? 0 : Object.create(require('qminer').la.Vector.prototype); }

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
    //# exports.SVR.prototype.predict = function(X) { return (X instanceof require('qminer').la.Vector | X instanceof require('qminer').la.SparseVector) ? 0 : Object.create(require('qminer').la.Vector.prototype); }

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
    //# exports.SVR.prototype.fit = function(X, y) { return Object.create(require('qminer').analytics.SVR.prototype); }
    JsDeclareFunction(fit);
};

/////////////////////////////////////////////
// Ridge Regression

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
//# exports.RidgeReg = function(arg) { return Object.create(require('qminer').analytics.RidgeReg.prototype) };
class TNodeJsRidgeReg : public node::ObjectWrap {
    friend class TNodeJsUtil;
public:
    static void Init(v8::Handle<v8::Object> exports);
    static const TStr GetClassId() { return "RidgeReg"; }
    ~TNodeJsRidgeReg() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; }

private:
    TRegression::TRidgeReg Model;

    TNodeJsRidgeReg(TSIn& SIn): Model(SIn) { }
    TNodeJsRidgeReg(const TRegression::TRidgeReg& _Model): Model(_Model) { }

    static TNodeJsRidgeReg* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

public:

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
    //# exports.RidgeReg.prototype.getParams = function () { return { gamma: 0.0 } }
    JsDeclareFunction(getParams);

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
    //# exports.RidgeReg.prototype.setParams = function (gamma) { return Object.create(require('qminer').analytics.RidgeReg.prototype); }
    JsDeclareFunction(setParams);

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
    //# exports.RidgeReg.prototype.fit = function(X, y) { return Object.create(require('qminer').analytics.RidgeReg.prototype); }
    JsDeclareFunction(fit);

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
    //# exports.RidgeReg.prototype.decisionFunction = function(X) { return 0.0; }

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
    //# exports.RidgeReg.prototype.predict = function(X) { return 0.0; }
    JsDeclareFunction(predict);

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
    //# exports.RidgeReg.prototype.weights = Object.create(require('qminer').la.Vector.prototype);
    JsDeclareProperty(weights);

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
    //# exports.RidgeReg.prototype.save = function(fout) { Object.create(require('qminer').fs.FOut.prototype); };
    JsDeclareFunction(save);
};

/////////////////////////////////////////////
// Sigmoid
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
//# exports.Sigmoid = function(arg) { return Object.create(require('qminer').analytics.Sigmoid.prototype); };
class TNodeJsSigmoid : public node::ObjectWrap {
    friend class TNodeJsUtil;
public:
    static void Init(v8::Handle<v8::Object> exports);
    static const TStr GetClassId() { return "Sigmoid"; }
    ~TNodeJsSigmoid() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; }

private:
    TSigmoid Sigmoid;

    TNodeJsSigmoid() {}
    TNodeJsSigmoid(TSIn& SIn): Sigmoid(SIn) {}

    static TNodeJsSigmoid* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

public:

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
    //# exports.Sigmoid.prototype.getParams = function () { return {}; }
    JsDeclareFunction(getParams);

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
    //# exports.Sigmoid.prototype.setParams = function (arg) { return Object.create(require('qminer').analytics.Sigmoid.prototype); }
    JsDeclareFunction(setParams);

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
    //# exports.Sigmoid.prototype.getModel = function () {return { A: 0, B: 0 }; }
    JsDeclareFunction(getModel);

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
    //# exports.Sigmoid.prototype.fit = function(X, y) { return Object.create(require('qminer').analytics.Sigmoid.prototype); }
    JsDeclareFunction(fit);

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
    //# exports.Sigmoid.prototype.decisionFunction = function(x) { return (x instanceof Object.create(require('qminer').la.Vector)) ? Object.create(require('qminer').la.Vector.prototype) : 0.0; }

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
    //# exports.Sigmoid.prototype.predict = function(x) { return (x instanceof Object.create(require('qminer').la.Vector)) ? Object.create(require('qminer').la.Vector.prototype) : 0.0; }
    JsDeclareFunction(predict);

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
    //# exports.Sigmoid.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); };
    JsDeclareFunction(save);
};

/////////////////////////////////////////////
// Nearest Neighbor Annomaly Detection

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
//# exports.NearestNeighborAD = function(arg) { return Object.create(require('qminer').analytics.NearestNeighborAD.prototype); };
class TNodeJsNNAnomalies : public node::ObjectWrap {
    friend class TNodeJsUtil;
public:
    static void Init(v8::Handle<v8::Object> exports);
    static const TStr GetClassId() { return "NearestNeighborAD"; }
    ~TNodeJsNNAnomalies() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; }

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
    //# exports.NearestNeighborAD.prototype.setParams = function (params) { return Object.create(require('qminer').analytics.NearestNeighborAD.prototype); }
    JsDeclareFunction(setParams);

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
    //# exports.NearestNeighborAD.prototype.getParams = function () { return { rate: 0.0, windowSize: 0.0 }; }
    JsDeclareFunction(getParams);

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
    //# exports.NearestNeighborAD.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); }
    JsDeclareFunction(save);

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
    //# exports.NearestNeighborAD.prototype.getModel = function () { return { rate: 0.1, threshold: 0.0 }; }
    JsDeclareFunction(getModel);

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
    //# exports.NearestNeighborAD.prototype.partialFit = function(X) { return Object.create(require('qminer').NearestNeighborAD.prototype); }
    JsDeclareFunction(partialFit);

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
    //# exports.NearestNeighborAD.prototype.predict = function(x) { return 0.0; }
    JsDeclareFunction(predict);

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
    //# exports.NearestNeighborAD.prototype.explain = function(x) { return {}; }
    JsDeclareFunction(explain);

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
    //# exports.NearestNeighborAD.prototype.init = false;
    JsDeclareProperty(init);
};

///////////////////////////////
////// code below not yet ported or verified for scikit
///////////////////////////////

///////////////////////////////
// QMiner-JavaScript-Recursive-Linear-Regression

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
//# exports.RecLinReg = function(arg) { return Object.create(require('qminer').analytics.RecLinReg.prototype); }
class TNodeJsRecLinReg : public node::ObjectWrap {
    friend class TNodeJsUtil;
private:
    TSignalProc::POnlineLinReg Model;
    TNodeJsRecLinReg(const TSignalProc::POnlineLinReg& Model);
public:
    static void Init(v8::Handle<v8::Object> exports);
    static const TStr GetClassId() { return "RecLinReg"; }
    ~TNodeJsRecLinReg() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; }
private:

    static TNodeJsRecLinReg* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

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
    //# exports.RecLinReg.prototype.partialFit = function (vec, num) { return Object.create(require('qminer').analytics.RecLinReg.prototype); }
    JsDeclareFunction(partialFit);

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
    //# exports.RecLinReg.prototype.fit = function (mat, vec) { return Object.create(require('qminer').analytics.RecLinReg.prototype); }
    JsDeclareFunction(fit);

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
    //# exports.RecLinReg.prototype.predict = function (vec) { return 0.0 }
    JsDeclareFunction(predict);

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
    //# exports.RecLinReg.prototype.setParams = function (params) { return Object.create(require('qminer').analytics.RecLinReg.prototype); }
    JsDeclareFunction(setParams);

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
    //# exports.RecLinReg.prototype.getParams = function () { return { dim: 0, regFact: 1.0, forgetFact: 1.0 }; }
    JsDeclareFunction(getParams);

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
    //# exports.RecLinReg.prototype.weights = Object.create(require('qminer').la.Vector.prototype);
    JsDeclareProperty(weights);

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
    //# exports.RecLinReg.prototype.dim = 0;
    JsDeclareProperty(dim);

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
    //# exports.RecLinReg.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); }
    JsDeclareFunction(save);

private:
    PJsonVal GetParams() const;
};

/////////////////////////////////////////////
// Logistic Regression

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
 * // create the input matrix and vector for fitting the model
 * var mat = new la.Matrix([[1, 0, -1, 0], [0, 1, 0, -1]]);
 * var vec = new la.Vector([1, 0, -1, -2]);
 * // if OpenBLAS is used, fit the model
 * if (require('qminer').flags.blas) {
 *     logreg.fit(mat, vec);
 *     // create the vector for the prediction
 *     var test = new la.Vector([1, 1]);
 *     // get the prediction
 *     var prediction = logreg.predict(test);
 * }
 */
//# exports.LogReg = function (arg) { return Object.create(require('qminer').analytics.LogReg.prototype); }
class TNodeJsLogReg : public node::ObjectWrap {
    friend class TNodeJsUtil;
public:
    static void Init(v8::Handle<v8::Object> exports);
    static const TStr GetClassId() { return "LogReg"; }
    ~TNodeJsLogReg() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; }

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
    //# exports.LogReg.prototype.setParams = function (param) { return Object.create(require('qminer').analytics.LogReg.prototype); }
    JsDeclareFunction(setParams);

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
    //# exports.LogReg.prototype.predict = function (x) { return 0.0; }
    JsDeclareFunction(predict);

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
    //# exports.LogReg.prototype.weights = Object.create(require('qminer').la.vector.prototype);
    JsDeclareProperty(weights);

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
    //# exports.LogReg.prototype.save = function (fout) { return Object.create(require('qminer').fs.FOut.prototype); }
    JsDeclareFunction(save);
};

/////////////////////////////////////////////
// Proportional Hazards Model

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
//# exports.PropHazards = function (arg) { return Object.create(require('qminer').analytics.PropHazards.prototype); }

class TNodeJsPropHaz : public node::ObjectWrap {
    friend class TNodeJsUtil;
public:
    static void Init(v8::Handle<v8::Object> exports);
    static const TStr GetClassId() { return "PropHazards"; }
    ~TNodeJsPropHaz() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; }

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
    * @returns {module:analytics.PropHazards} Self. The model parameters have been updated.
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
    //# exports.PropHazards.prototype.fit = function(X, y, eps) { return Object.create(require('qminer').analytics.PropHazards.prototype); }
    JsDeclareFunction(fit);

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
    //# exports.PropHazards.prototype.predict = function(x) { return 0.0; }
    JsDeclareFunction(predict);

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
    //# exports.PropHazards.prototype.weights = Object.create(require('qminer').la.Vector.prototype);
    JsDeclareProperty(weights);

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
    //# exports.PropHazards.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); }
    JsDeclareFunction(save);
};


///////////////////////////////
// QMiner-JavaScript-Neural-Networks

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
* // create the matrices for the fitting of the model
* var matIn = new la.Matrix([[1, 0], [0, 1], [1, 1]]);
* var matOut = new la.Matrix([[-1, 8], [-3, -3]]);
* // fit the model
* nnet.fit(matIn, matOut);
* // create the vector for the prediction
* var test = new la.Vector([1, 1, 2]);
* // predict the value of the vector
* var prediction = nnet.predict(test);
*/
//# exports.NNet = function (arg) { return Object.create(require('qminer').analytics.NNet.prototype); }
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
    ~TNodeJsNNet() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; }

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
    //# exports.NNet.prototype.getParams = function () { return { layout: Object.create(require('qminer').la.IntVector.prototype), learnRate: 0.0, momentum: 0.0, tFuncHidden: "", TFuncOut: "" }; }
    JsDeclareFunction(getParams);

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
    //# exports.NNet.prototype.setParams = function (params) { return Object.create(require('qminer').analytics.NNet.prototype); }
    JsDeclareFunction(setParams);

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
    //# exports.NNet.prototype.fit = function (input, output) { return Object.create(require('qminer').analytics.NNet.prototype); }
    JsDeclareFunction(fit);

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
    //# exports.NNet.prototype.predict = function (vec) { return Object.create(require('qminer').la.Vector.prototype); }
    JsDeclareFunction(predict);

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
    //# exports.NNet.prototype.save = function (fout) { return Object.create(require('qminer').fs.FOut.prototype); }
    JsDeclareFunction(save);
 private:
    TSignalProc::TTFunc ExtractFuncFromString(const TStr& FuncString);
};

/////////////////////////////////////////////
// QMiner-JavaScript-Tokenizer

/**
* @typedef {Object} tokenizerParam
* An object used for the construction of {@link module:analytics.Tokenizer}.
* @property {string} [type='unicode'] - The type of the tokenizer. The different types are:
*<br>1. 'simple' - Creates break on white spaces.
*<br>2. 'html' - Creates break on white spaces and ignores html tags.
*<br>3. 'unicode' - Creates break on white spaces and normalizes unicode letters, e.g. �� changes to cs�z.
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
//# exports.Tokenizer = function (arg) { return Object.create(require("qminer").analytics.Tokenizer.prototype); }
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
    ~TNodeJsTokenizer() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; }
    static TNodeJsTokenizer* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

    // Functions:
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
    //# exports.Tokenizer.prototype.getTokens = function (str) { return [""]; }
    JsDeclareFunction(getTokens);

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
    //# exports.Tokenizer.prototype.getSentences = function (str) { return [""]; }
    JsDeclareFunction(getSentences);

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
    //# exports.Tokenizer.prototype.getParagraphs = function (str) { return [""]; }
    JsDeclareFunction(getParagraphs);

};

/////////////////////////////////////////////
// QMiner-JavaScript-Multidimensional Scaling

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
* // create the multidimensional matrix
* var mat = new la.Matrix({ rows: 50, cols: 10, random: true });
* // get the 2d representation of mat
* var mat2d = mds.fitTransform(mat);
*/
//# exports.MDS = function (arg) { return Object.create(require('qminer').analytics.MDS.prototype); }
class TNodeJsMDS : public node::ObjectWrap {
    friend class TNodeJsUtil;
public:
    static void Init(v8::Handle<v8::Object> exports);
    static const TStr GetClassId() { return "MDS"; }
    ~TNodeJsMDS() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; }

private:
    // parameters
    int MxStep, MxSecs;
    double MnDiff;
    TVizDistType DistType;

    TNodeJsMDS(const PJsonVal& ParamVal);
    TNodeJsMDS(TSIn& SIn);

    static TNodeJsMDS* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

private:
    class TFitTransformTask: public TNodeTask {
        TNodeJsMDS* JsMDS;
        TNodeJsFltVV* JsFltVV;
        TNodeJsSpMat* JsSpVV;
        TNodeJsFltVV* JsResult;
        PNotify Notify;

    public:
        TFitTransformTask(const v8::FunctionCallbackInfo<v8::Value>& Args);

        v8::Handle<v8::Function> GetCallback(const v8::FunctionCallbackInfo<v8::Value>& Args);
        void Run();
        v8::Local<v8::Value> WrapResult();
    };

public:
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
    //# exports.MDS.prototype.getParams = function () { return { maxStep: 0, maxSecs: 0, minDiff: 0, distType: "" }; }
    JsDeclareFunction(getParams);

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
    //# exports.MDS.prototype.setParams = function (params) { return Object.create(require('qminer').analytics.MDS.prototype); }
    JsDeclareFunction(setParams);

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
    //# exports.MDS.prototype.fitTransform = function (mat, callback) { return Object.create(require('qminer').la.Matrix.prototype); }
    JsDeclareSyncAsync(fitTransform, fitTransformAsync, TFitTransformTask);

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
    //# exports.MDS.prototype.save = function (fout) { return Object.create(require('qminer').fs.FOut.prototype); }
    JsDeclareFunction(save);

private:
    void UpdateParams(const PJsonVal& ParamVal);
    PJsonVal GetParams() const;
    void Save(TSOut& SOut) const;
};


/////////////////////////////////////////////
// QMiner-JavaScript-KMeans

/**
* @typedef {Object} KMeansParam
* An object used for the construction of {@link module:analytics.KMeans}.
* @property {number} [iter=10000] - The maximum number of iterations.
* @property {number} [k=2] - The number of centroids.
* @property {boolean} [allowEmpty=true] - Whether to allow empty clusters to be generated.
* @property {boolean} [calcDistQual=false] - Whether to calculate the quality measure based on distance, if false relMeanCentroidDist will return 'undefined'
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
* // predict where the columns of the matrix will be assigned
* var Y = new la.Matrix([[1, 1, 0], [-2, 3, 1]]);
* var prediction = KMeans.predict(Y);
*/
//# exports.KMeans = function (arg) { return Object.create(require('qminer').analytics.KMeans.prototype); }
class TNodeJsKMeans : public node::ObjectWrap {
    friend class TNodeJsUtil;
public:
    static void Init(v8::Handle<v8::Object> exports);
    static const TStr GetClassId() { return "KMeans"; }

private:
    enum TDistanceType { dtEuclid, dtCos };
    enum TCentroidType { ctDense, ctSparse };

    int Iter;
    int K;
    TBool AllowEmptyP;
    TBool CalcDistQualP {false};

    TIntV AssignV;
    TIntV Medoids;

    TIntV FitIdx;
    TFltVV DenseFitMatrix;
    TVec<TIntFltKdV> SparseFitMatrix;

    TDistanceType DistType;
    TClustering::PDist Dist;

    TCentroidType CentType;
    void* Model;

    bool Verbose;
    PNotify Notify;

    TNodeJsKMeans(const PJsonVal& ParamVal);
    TNodeJsKMeans(const PJsonVal& ParamVal, const TFltVV& Mat);
    TNodeJsKMeans(const PJsonVal& ParamVal, const TVec<TIntFltKdV>& Mat);
    TNodeJsKMeans(TSIn& SIn);
    ~TNodeJsKMeans();

    static TNodeJsKMeans* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

private:
    class TFitTask : public TNodeTask {
        TNodeJsKMeans* JsKMeans;
        // first argument
        TNodeJsFltVV*  JsFltVV;
        TNodeJsSpMat*  JsSpVV;
        // second argument
        TNodeJsIntV*   JsIntV;
        TNodeJsIntV*   JsArr;

    public:
        TFitTask(const v8::FunctionCallbackInfo<v8::Value>& Args);

        v8::Handle<v8::Function> GetCallback(const v8::FunctionCallbackInfo<v8::Value>& Args);
        void Run();
    };

public:

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
    //# exports.KMeans.prototype.getParams = function () { return { iter: 10000, k: 2, distanceType: "Euclid", centroidType: "Dense", verbose: false }; }
    JsDeclareFunction(getParams);

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
    //# exports.KMeans.prototype.setParams = function (params) { return Object.create(require('qminer').analytics.KMeans.prototype); }
    JsDeclareFunction(setParams);

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
    //# exports.KMeans.prototype.fit = function (X) { return Object.create(require('qminer').analytics.KMeans.prototype); }
    JsDeclareSyncAsync(fit, fitAsync, TFitTask);

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
    //# exports.KMeans.prototype.predict = function (A) { return Object.create(require('qminer').la.IntVector.prototype); }
    JsDeclareFunction(predict);

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
    //# exports.KMeans.prototype.transform = function (A) { return Object.create(require('qminer').la.Matrix.prototype); }
    JsDeclareFunction(transform);

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
    //# exports.KMeans.prototype.permuteCentroids = function (mapping) { return Object.create(require('qminer').analytics.KMeans.prototype); }
    JsDeclareFunction(permuteCentroids);

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
    //# exports.KMeans.prototype.save = function (fout) { return Object.create(require('qminer').fs.FOut.prototype); }
    JsDeclareFunction(save);

    /**
     * The centroids created with the fit method. Type {@link module:la.Matrix}.
     * @example
     * // import the modules
     * var analytics = require('qminer').analytics;
     * // create a new KMeans object
     * var KMeans = new analytics.KMeans({ iter: 1000, k: 3 });
     * // get the centroids
     * var centroids = KMeans.centroids;
     * // print the first centroid
     * console.log(centroids.getCol(0));
     */
    //# exports.KMeans.prototype.centroids = Object.create(require('qminer').la.Matrix.prototype);
    JsDeclareProperty(centroids);

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
    //# exports.KMeans.prototype.medoids = Object.create(require('qminer').la.IntVector.prototype);
    JsDeclareProperty(medoids);

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
    //# exports.KMeans.prototype.idxv = Object.create(require('qminer').la.IntVector.prototype);
    JsDeclareProperty(idxv);

    /**
     * Returns the normalized weighted distance between the vectors and their centroids
     * using the following formula:
     *  d = \frac{sum_i p_i*sum_j d(x_j,c_i) / n_i}{sum_{k=1}^n d(x_k, mu) / n}
     *    = \frac{sum_{i,j} d(c_i,x_j)}{sum_k d(x_k, mu)}
     *
     * @returns {number} relMeanDist
     */
    //# exports.KMeans.prototype.relMeanCentroidDist = 0;
    JsDeclareProperty(relMeanCentroidDist);

private:
    void UpdateParams(const PJsonVal& ParamVal);
    void Save(TSOut& SOut) const;
    void CleanUp();
};


/////////////////////////////////////////////
// QMiner-JavaScript-DpMeans

/**
* @typedef {Object} DpMeansParam
* An object used for the construction of {@link module:analytics.KMeans}.
* @property {number} [iter=10000] - The maximum number of iterations.
* @property {number} [lambda=1] - Maximum radius of the clusters
* @property {number} [minClusters=2] - Minimum number of clusters
* @property {number} [maxClusters=inf] - Maximum number of clusters
* @property {boolean} [allowEmpty=true] - Whether to allow empty clusters to be generated.
* @property {boolean} [calcDistQual=false] - Whether to calculate the quality measure based on distance, if false relMeanCentroidDist will return 'undefined'
* @property {string} [centroidType="Dense"] - The type of centroids. Possible options are `'Dense'` and `'Sparse'`.
* @property {string} [distanceType="Euclid"] - The distance type used at the calculations. Possible options are `'Euclid'` and `'Cos'`.
* @property {boolean} [verbose=false] - If `false`, the console output is supressed.
* @property {Array.<number>} [fitIdx] - The index array used for the construction of the initial centroids.
* @property {Object} [fitStart] - The KMeans model returned by {@link module:analytics.KMeans.prototype.getModel} used for centroid initialization.
* @property {(module:la.Matrix | module:la.SparseMatrix)} fitStart.C - The centroid matrix.
*/

/**
 * DpMeans Clustering
 * @classdesc DpMeans Clustering is an iterative, data-partitioning algorithm that assigns observations into clusters with the nearest centroid according to some metric.
 *              The number of clusters is not known in advance, but can be upper and lower bounded. Rather the parameter
 *              is the maximum radius of a cluster `lambda`.
 * @class
 * @param {module:analytics~DpMeansParam | module:fs.FIn} [arg] - Construction arguments. There are two ways of constructing:
 * <br>1. Using the {@link module:analytics~DpMeansParam} object,
 * <br>2. using the file input stream {@link module:fs.FIn}.
 * @example
 * // import analytics and la modules
 * var analytics = require('qminer').analytics;
 * var la = require('qminer').la;
 * // create a KMeans object
 * var dpmeans = new analytics.DpMeans();
 * // create the matrix to be fitted
 * var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
 * // create the model
 * dpmeans.fit(X);
 * // predict where the columns of the matrix will be assigned
 * var Y = new la.Matrix([[1, 1, 0], [-2, 3, 1]]);
 * var prediction = dpmeans.predict(Y);
 */
//# exports.DpMeans = function (arg) { return Object.create(require('qminer').analytics.DpMeans.prototype); }
class TNodeJsDpMeans : public node::ObjectWrap {
    friend class TNodeJsUtil;
public:
    static void Init(v8::Handle<v8::Object> exports);
    static const TStr GetClassId() { return "DpMeans"; }

    using TDenseModel = TClustering::TDpMeans<TFltVV>;
    using TSparseModel = TClustering::TDpMeans<TVec<TIntFltKdV>>;

private:
    enum class TDistanceType { dtEuclid, dtCos };
    enum class TCentroidType { ctDense, ctSparse };

    int Iter;

    TFlt Lambda {1};
    TInt MnClusts {2};
    TInt MxClusts {TInt::Mx};

    TBool AllowEmptyP;
    TBool CalcDistQualP {false};

    TIntV AssignV;
    TIntV Medoids;

    TIntV FitIdx;
    TFltVV DenseFitMatrix;
    TVec<TIntFltKdV> SparseFitMatrix;

    TDistanceType DistType;
    TClustering::PDist Dist;

    TCentroidType CentType;
    void* DpMeansModel;

    bool Verbose;
    PNotify Notify;

    TNodeJsDpMeans(const PJsonVal& ParamVal);
    TNodeJsDpMeans(const PJsonVal& ParamVal, const TFltVV& Mat);
    TNodeJsDpMeans(const PJsonVal& ParamVal, const TVec<TIntFltKdV>& Mat);
    TNodeJsDpMeans(TSIn& SIn);
    ~TNodeJsDpMeans();

    static TNodeJsDpMeans* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

private:
    class TFitTask : public TNodeTask {
        TNodeJsDpMeans* JsDpMeans {nullptr};
        // first argument
        TNodeJsFltVV*  JsFltVV {nullptr};
        TNodeJsSpMat*  JsSpVV {nullptr};
        // second argument
        TNodeJsIntV*   JsIntV {nullptr};
        TNodeJsIntV*   JsArr {nullptr};

    public:
        TFitTask(const v8::FunctionCallbackInfo<v8::Value>& Args);

        v8::Handle<v8::Function> GetCallback(const v8::FunctionCallbackInfo<v8::Value>& Args);
        void Run();
    };

public:

    /**
    * Returns the parameters.
    * @returns {module:analytics~KMeansParam} The construction parameters.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create a new DpMeans object
    * var DpMeans = new analytics.DpMeans({ iter: 1000, lambda: 5 });
    * // get the parameters
    * var json = DpMeans.getParams();
    * console.log(json.lambda);
    */
    //# exports.DpMeans.prototype.getParams = function () { return { iter: 10000, lambda: 2, distanceType: "Euclid", centroidType: "Dense", verbose: false }; }
    JsDeclareFunction(getParams);

    /**
     * Sets the parameters.
     * @param {module:analytics~KMeansParam} params - The construction parameters.
     * @returns {module:analytics.DpMeans} Self. The model parameters have been updated.
     * @example
     * // import analytics module
     * var analytics = require('qminer').analytics;
     * // create a new DpMeans object
     * var DpMeans = new analytics.DpMeans();
     * // change the parameters of the DpMeans object
     * DpMeans.setParams({ iter: 1000, lambda: 5 });
     */
    //# exports.DpMeans.prototype.setParams = function (params) { return Object.create(require('qminer').analytics.DpMeans.prototype); }
    JsDeclareFunction(setParams);

    /**
     * Calculates the centroids.
     * @param {module:la.Matrix | module:la.SparseMatrix} X - Matrix whose columns correspond to examples.
     * @returns {module:analytics.DpMeans} Self. The model has been updated.
     * @example <caption> Asynchronous function </caption>
     * // import analytics module
     * var analytics = require('qminer').analytics;
     * var la = require('qminer').la;
     * // create a new DpMeans object
     * var DpMeans = new analytics.DpMeans({ iter: 1000, lambda: 3 });
     * // create a matrix to be fitted
     * var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
     * // create the model with the matrix X
     * DpMeans.fitAsync(X, function (err) {
     *     if (err) console.log(err);
     *     // successful calculation
     * });
     *
     * @example <caption> Synchronous function </caption>
     * var analytics = require('qminer').analytics;
     * var la = require('qminer').la;
     * // create a new DpMeans object
     * var DpMeans = new analytics.DpMeans({ iter: 1000, lambda: 3 });
     * // create a matrix to be fitted
     * var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
     * // create the model with the matrix X
     * DpMeans.fit(X);
     */
    //# exports.DpMeans.prototype.fit = function (X) { return Object.create(require('qminer').analytics.DpMeans.prototype); }
    JsDeclareSyncAsync(fit, fitAsync, TFitTask);

    /**
     * Returns an vector of cluster id assignments.
     * @param {module:la.Matrix | module:la.SparseMatrix} A - Matrix whose columns correspond to examples.
     * @returns {module:la.IntVector} Vector of cluster assignments.
     * @example
     * // import analytics module
     * var analytics = require('qminer').analytics;
     * var la = require('qminer').la;
     * // create a new DpMeans object
     * var DpMeans = new analytics.DpMeans({ iter: 1000, lambda: 3 });
     * // create a matrix to be fitted
     * var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
     * // create the model with the matrix X
     * DpMeans.fit(X);
     * // create the matrix of the prediction vectors
     * var pred = new la.Matrix([[2, -1, 1], [1, 0, -3]]);
     * // predict the values
     * var prediction = DpMeans.predict(pred);
     */
    //# exports.DpMeans.prototype.predict = function (A) { return Object.create(require('qminer').la.IntVector.prototype); }
    JsDeclareFunction(predict);

    /**
     * Transforms the points to vectors of distances to centroids.
     * @param {module:la.Matrix | module:la.SparseMatrix} A - Matrix whose columns correspond to examples.
     * @returns {module:la.Matrix} Matrix where each column represents the squared distances to the centroid vectors.
     * @example
     * // import modules
     * var analytics = require('qminer').analytics;
     * var la = require('qminer').la;
     * // create a new DpMeans object
     * var DpMeans = new analytics.DpMeans({ iter: 1000, lambda: 3 });
     * // create a matrix to be fitted
     * var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
     * // create the model with the matrix X
     * DpMeans.fit(X);
     * // create the matrix of the transform vectors
     * var matrix = new la.Matrix([[-2, 0], [0, -3]]);
     * // get the transform values of matrix
     * // returns the matrix
     * //  10    17
     * //   1    20
     * //  10     1
     * DpMeans.transform(matrix);
     */
    //# exports.DpMeans.prototype.transform = function (A) { return Object.create(require('qminer').la.Matrix.prototype); }
    JsDeclareFunction(transform);

    /**
     * Permutates the clusters, and with it {@link module:analytics.DpMeans#centroids}, {@link module:analytics.DpMeans#medoids} and {@link module:analytics.DpMeans#idxv}.
     * @param {module:la.IntVector} mapping - The mapping, where `mapping[4] = 2` means "map cluster 4 into cluster 2".
     * @returns {module:analytics.DpMeans} Self. The clusters has been permutated.
     * @example
     * // import the modules
     * var analytics = require('qminer').analytics;
     * var la = require('qminer').la;
     * // create a new DpMeans object
     * var DpMeans = new analytics.DpMeans({ iter: 1000, lambda: 3 });
     * // create a matrix to be fitted
     * var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
     * // create the model with the matrix X
     * DpMeans.fit(X);
     * // create the mapping vector
     * var Mapping = new la.IntVector([1, 0, 2]);
     * // permutate the clusters.
     * DpMeans.permuteCentroids(Mapping);
     */
    //# exports.DpMeans.prototype.permuteCentroids = function (mapping) { return Object.create(require('qminer').analytics.DpMeans.prototype); }
    JsDeclareFunction(permuteCentroids);

    /**
     * Saves DpMeans internal state into (binary) file.
     * @param {module:fs.FOut} fout - The output stream.
     * @returns {module:fs.FOut} The output stream `fout`.
     * @example
     * // import the modules
     * var analytics = require('qminer').analytics;
     * var la = require('qminer').la;
     * var fs = require('qminer').fs;
     * // create a new DpMeans object
     * var DpMeans = new analytics.DpMeans({ iter: 1000, lambda: 3 });
     * // create a matrix to be fitted
     * var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
     * // create the model with the matrix X
     * DpMeans.fit(X);
     * // create the file output stream
     * var fout = new fs.openWrite('DpMeans.bin');
     * // save the DpMeans instance
     * DpMeans.save(fout);
     * fout.close();
     * // load the DpMeans instance
     * var fin = fs.openRead('DpMeans.bin');
     * var KMeans2 = new analytics.DpMeans(fin);
     */
    //# exports.DpMeans.prototype.save = function (fout) { return Object.create(require('qminer').fs.FOut.prototype); }
    JsDeclareFunction(save);

    /**
     * The centroids created with the fit method. Type {@link module:la.Matrix}.
     * @example
     * // import the modules
     * var analytics = require('qminer').analytics;
     * // create a new DpMeans object
     * var DpMeans = new analytics.DpMeans({ iter: 1000, lambda: 3 });
     * // get the centroids
     * var centroids = DpMeans.centroids;
     * // print the first centroid
     * console.log(centroids.getCol(0));
     */
    //# exports.DpMeans.prototype.centroids = Object.create(require('qminer').la.Matrix.prototype);
    JsDeclareProperty(centroids);

    /**
    * The medoids created with the fit method. Type {@link module:la.IntVector}.
    * @example
    * // import the modules
    * var analytics = require('qminer').analytics;
    * // create a new DpMeans object
    * var DpMeans = new analytics.DpMeans({ iter: 1000, lambda: 3 });
    * // get the centroids
    * var medoids = DpMeans.medoids;
    */
    //# exports.DpMeans.prototype.medoids = Object.create(require('qminer').la.IntVector.prototype);
    JsDeclareProperty(medoids);

    /**
    * The integer vector containing the cluster ids of the training set created with the fit method. Type {@link module:la.IntVector}.
    * @example
    * // import the modules
    * var analytics = require('qminer').analytics;
    * // create a new DpMeans object
    * var DpMeans = new analytics.DpMeans({ iter: 1000, lambda: 3 });
    * // get the idxv
    * var idxv = DpMeans.idxv;
    */
    //# exports.DpMeans.prototype.idxv = Object.create(require('qminer').la.IntVector.prototype);
    JsDeclareProperty(idxv);

    /**
     * Returns the normalized weighted distance between the vectors and their centroids
     * using the following formula:
     *  d = \frac{sum_i p_i*sum_j d(x_j,c_i) / n_i}{sum_{k=1}^n d(x_k, mu) / n}
     *    = \frac{sum_{i,j} d(c_i,x_j)}{sum_k d(x_k, mu)}
     *
     * @returns {number} relMeanDist
     */
    //# exports.DpMeans.prototype.relMeanCentroidDist = 0;
    JsDeclareProperty(relMeanCentroidDist);

private:
    void UpdateParams(const PJsonVal& ParamVal);
    void Save(TSOut& SOut) const;
    void CleanUp();

    const TDenseModel* GetDenseModel() const;
    const TSparseModel* GetSparseModel() const;
    TDenseModel* GetDenseModel();
    TSparseModel* GetSparseModel();

    int GetClusts() const;
};

/////////////////////////////////////////////
// QMiner-JavaScript-TDigest

/**
* @typedef {Object} TDigestParam
* An object used for the construction of {@link module:analytics.TDigest}.
* @property {number} [minCount=0] - The minimal number of examples before the model is initialized.
* @property {number} [clusters=100] - The number of 1-d clusters (large values lead to higher memory usage).
*/

/**
* TDigest quantile estimation on streams
* @classdesc TDigest is a methods that approximates the CDF function of streaming measurements.
*   Data structure useful for percentile and quantile estimation for online data streams.
*   It can be added to any anomaly detector to set the number of alarms triggered as a percentage of the total samples.
*   Adding new samples to the distribution is achieved through `partialFit` and querying the model (computing quantiles)
*   is implemented by `predict` function.
*   <br> This is based on the Data Lib Sketch Implementation: {@link https://github.com/vega/datalib-sketch/blob/master/src/t-digest.js t-digest.js}
*   <br> Paper: Ted Dunning, Otmar Ertl - {@link https://github.com/tdunning/t-digest/blob/master/docs/t-digest-paper/histo.pdf Computing Extremely Accurate Quantiles Using t-Digests}.
* @class
* @param {module:analytics~TDigestParam | module:fs.FIn} [arg] - Construction arguments. There are two ways of constructing:
* <br>1. Using the {@link module:analytics~TDigestParam} object,
* <br>2. using the file input stream {@link module:fs.FIn}.
* @example
* // import modules
* var qm = require('qminer');
* var analytics = qm.analytics;
* // create the default TDigest object
* var tdigest = new analytics.TDigest();
* // create the data used for calculating quantiles
* var inputs = [10, 1, 2, 8, 9, 5, 6, 4, 7, 3];
* // fit the TDigest model
* for (var i = 0; i < inputs.length; i++) {
*     tdigest.partialFit(inputs[i]);
* }
* // make the prediction for the 0.1 quantile
* var prediction = tdigest.predict(0.1);
* // save the model
* tdigest.save(fs.openWrite('tdigest.bin')).close();
* // open the tdigest model under a new variable
* var tdigest2 = new analytics.TDigest(fs.openRead('tdigest.bin'));
*/
//# exports.TDigest = function (arg) { return Object.create(require('qminer').analytics.TDigest.prototype); }
class TNodeJsTDigest : public node::ObjectWrap {
    friend class TNodeJsUtil;
public:
    static void Init(v8::Handle<v8::Object> exports);
    static const TStr GetClassId() { return "TDigest"; }

private:
    TSignalProc::TTDigest Model;

    TNodeJsTDigest(const PJsonVal& ParamVal);
    TNodeJsTDigest(TSIn& SIn);
    ~TNodeJsTDigest() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; }

    static TNodeJsTDigest* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

public:

    /**
    * Returns the parameters.
    * @returns {module:analytics~TDigestParam} The construction parameters.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var analytics = qm.analytics;
    * // create the default TDigest object
    * var tdigest = new analytics.TDigest();
    * // get the parameters of the object
    * var params = tdigest.getParams();
    */
    //# exports.TDigest.prototype.getParams = function () { return { }; }
    JsDeclareFunction(getParams);

    /**
    * Sets the parameters.
    * @param {module:analytics~TDigestParam} params - The construction parameters.
    * @returns {module:analytics.TDigest} Self. The model parameters have been updated.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var analytics = qm.analytics;
    * // create the default TDigest object
    * var tdigest = new analytics.TDigest();
    * // set the parameters of the object
    * var params = tdigest.setParams({ minCount: 10, clusters: 50 });
    */
    //# exports.TDigest.prototype.setParams = function (params) { return Object.create(require('qminer').analytics.TDigest.prototype); }
    JsDeclareFunction(setParams);

    /**
    * Adds a new measurement to the model and updates the approximation of the data distribution.
    * @param {number} x - Input number.
    * @returns {module:analytics.TDigest} Self. The model has been updated.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var analytics = qm.analytics;
    * // create the default TDigest object
    * var tdigest = new analytics.TDigest();
    * // create the data used for calculating quantiles
    * var inputs = [10, 1, 2, 8, 9, 5, 6, 4, 7, 3];
    * // fit the TDigest model with all input values
    * for (var i = 0; i < inputs.length; i++) {
    *     tdigest.partialFit(inputs[i]);
    * }
    */
    //# exports.TDigest.prototype.partialFit = function (X) { return Object.create(require('qminer').analytics.TDigest.prototype); }
    JsDeclareFunction(partialFit);

    /**
    * Returns a quantile given input number, that is the approximate fraction of samples smaller than the input (0.05 means that 5% of data is smaller than the input value).
    * @param {number} x - Input number.
    * @returns {number} Quantile (between 0 and 1).
    * @example
    * // import modules
    * var qm = require('qminer');
    * var analytics = qm.analytics;
    * // create the default TDigest object
    * var tdigest = new analytics.TDigest();
    * // create the data used for calculating quantiles
    * var inputs = [10, 1, 2, 8, 9, 5, 6, 4, 7, 3];
    * // fit the TDigest model
    * for (var i = 0; i < inputs.length; i++) {
    *     tdigest.partialFit(inputs[i]);
    * }
    * // make the prediction for the 0.1 quantile
    * var prediction = tdigest.predict(0.1);
    */
    //# exports.TDigest.prototype.predict = function (x) { return 0; }
    JsDeclareFunction(predict);

    /**
    * Saves TDigest internal state into (binary) file.
    * @param {module:fs.FOut} fout - The output stream.
    * @returns {module:fs.FOut} The output stream `fout`.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var analytics = qm.analytics;
    * var fs = qm.fs;
    * // create the default TDigest object
    * var tdigest = new analytics.TDigest();
    * // create the data used for calculating quantiles
    * var inputs = [10, 1, 2, 8, 9, 5, 6, 4, 7, 3];
    * // fit the TDigest model
    * for (var i = 0; i < inputs.length; i++) {
    *     tdigest.partialFit(inputs[i]);
    * }
    * // save the model
    * tdigest.save(fs.openWrite('tdigest.bin')).close();
    * // open the tdigest model under a new variable
    * var tdigest2 = new analytics.TDigest(fs.openRead('tdigest.bin'));
    */
    //# exports.TDigest.prototype.save = function (fout) { return Object.create(require('qminer').fs.FOut.prototype); }
    JsDeclareFunction(save);

    /**
    * Returns true when the model has enough data to initialize. Type `boolean`.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var analytics = qm.analytics;
    * var fs = qm.fs;
    * // create the default TDigest object
    * var tdigest = new analytics.TDigest();
    * // check if the model has enough data to initialize
    * if (tdigest.init) { console.log("Ready to initialize"); }
    */
    //# exports.TDigest.prototype.init = false;
    JsDeclareProperty(init);

};

/**
* @typedef {Object} GkParam
* An object used for the construction of {@link module:analytics.Gk}.
* @property {number} [eps=0.01] - Determines the relative error of the algorithm.
* @property {boolean} [autoCompress=true] - Whether the summary should be compresses automatically or manually.
*/

/**
 * @classdesc Greenwald - Khanna algorithm for online quantile estimation. Given
 *   a comulative probability p, the algorithm returns the approximate value of
 *   the p-th quantile.
 *
 *   The algorithm works by keeping a summary of buckets, each summarizing a
 *   range of values. Through the run of the algorithm new buckets are created
 *   and periodically merged if possible.
 *
 *   It is was first explained in:
 *   "Space-Efficient Online Computation of Quantile Summaries"
 *   http://infolab.stanford.edu/~datar/courses/cs361a/papers/quantiles.pdf
 *
 *   The error is bounded by the rank of the output element (not by the absolute value).
 *   Specifically, the worst case error in rank is bounded by eps*n, where n is the
 *   number of elements in the summary.
 *
 * @class
 * @param {module:analytics~GkParam | module:fs.FIn} [arg] - Construction arguments. There are two ways of constructing:
 * <br>1. Using the {@link module:analytics~GkParam} object,
 * <br>2. using the file input stream {@link module:fs.FIn}.
 *
 * @example
 * // import modules
 * var qm = require('qminer');
 * var analytics = qm.analytics;
 *
 * // create the default TDigest object
 * var gk = new analytics.Gk({
 *     eps: 0.001,
 *     autoCompress: true
 * });
 *
 * // create the data used for calculating quantiles
 * var inputs = [10, 1, 2, 8, 9, 5, 6, 4, 7, 3];
 *
 * // fit the TDigest model
 * for (var i = 0; i < inputs.length; i++) {
 *     gk.partialFit(inputs[i]);
 * }
 *
 * // make the prediction for the 0.1 quantile
 * var prediction = gk.predict(0.1);
 * // save the model
 * gk.save(fs.openWrite('gk.bin')).close();
 * // open the gk model under a new variable
 * var gk2 = new analytics.Gk(fs.openRead('gk.bin'));
 */
//# exports.Gk = function (arg) { return Object.create(require('qminer').analytics.Gk.prototype); }
class TNodeJsGk : public node::ObjectWrap {
    friend class TNodeJsUtil;
public:
    static void Init(v8::Handle<v8::Object> exports);
    static const TStr GetClassId() { return "Gk"; }

private:
    TQuant::TGk Gk;

    TNodeJsGk(const PJsonVal&);
    TNodeJsGk(TSIn&);
    ~TNodeJsGk() {}

    static TNodeJsGk* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

public:
    /**
     * Returns the models' parameters as a JavaScript object (JSON). These parameters
     * are the same as are set through the constructor.
     *
     * @returns {module:analytics~GkParam} The construction parameters.
     *
     * var analytics = qm.analytics;
     * var gk = new analytics.Gk();
     * var params = gk.getParams();
     *
     * console.log(params.eps);
     * console.log(params.autoCompress);
     */
    //# exports.Gk.prototype.getParams = function () { return { }; }
    JsDeclareFunction(getParams);

    /**
     * Adds a new value to the summary.
     *
     * @param {number} val - the value
     * @returns {module:analytics.Gk} reference to self
     *
     * @example
     * var qm = require('qminer');
     *
     * var gk = new qm.analytics.CountWindowGk();
     * gk.partialFit(1.0);
     * gk.partialFit(2.0);
     */
    //# exports.Gk.compress = function (fout) { return Object.create(require('qminer').analytics.Gk.prototype); }
    JsDeclareFunction(partialFit);

    /**
     * Given an input cumulative probability, returns a quantile associated with that
     * probability (e.g. for input 0.5 it will return the median).
     *
     * @param {number|Array} pVals - the p-values which we a querying
     * @returns {number|Array} quantiles - depending whether the input was a single value or array the method returns a quantile or array of quantiles
     *
     * @example
     * var qm = require('qminer');
     *
     * var gk = new qm.analytics.Gk({
     *     eps: 0.1
     * });
     * gk.partialFit(1.0);
     * gk.partialFit(2.0);
     * gk.partialFit(1.0);
     * gk.partialFit(3.0);
     * gk.partialFit(2.0);
     *
     * console.log(gk.predict(0.01));   // prints the first percentile
     * console.log(gk.predict(0.25));   // prints the first quartile
     * console.log(gk.predict(0.5));    // prints the median
     */
    //# exports.Gk.prototype.predict = function (x) { return 0; }
    JsDeclareFunction(predict);

    /**
     * Manually runs the compression procedure.
     *
     * @returns reference to self
     */
    //# exports.Gk.compress = function (fout) { return Object.create(require('qminer').analytics.Gk.prototype); }
    JsDeclareFunction(compress);

    /**
     * Saves the objects state into the output stream.
     *
     * @param {module:fs.FOut} fout - the output stream
     * @returns {module:fs.FOut} - the output stream
     */
    //# exports.Gk.save = function (fout) { return Object.create(require('qminer').fs.FOut.prototype); }
    JsDeclareFunction(save);
};


/**
* @typedef {Object} BiasedGkParam
* An object used for the construction of {@link module:analytics.BiasedGk}.
* @property {number} [targetProb=0.01] - The probability where the algorithm is most accurate. Its accuracy is determined as eps*max(p, targetProb) when targetProb < 0.5 and eps*max(1-p, 1-targetProb) when targetProb >= 0.5. Higher values of `targetProb` allow for a smaller memory footprint.
* @property {number} [eps=0.1] - Parameter which determines the accuracy.
* @property {string} [compression="periodic"] - Determines when the algorithm compresses its summary. Options are: "periodic", "aggressive" and "manual".
* @property {boolean} [useBands=true] - Whether the algorithm should use the 'band' subprocedure. Using this subprocedure should result in a smaller summary.
*/

/**
 * @classdesc The CKMS (GK adapted for biased quantiles) algorithm for online
 *   biased quantile estimation. Given a probability p the algorithm returns
 *   the approximate value of the p-th quantile. The algorithm is most accurate
 *   in one of the extremes (which extreme depends on the parameters).
 *
 *   The algorithm works by keeping a summary of buckets, each summarizing a
 *   range of values. Through the run of the algorithm new buckets are created
 *   and periodically merged if possible.
 *
 *   It was first explained in:
 *   "Effective Computation of Biased Quantiles over Data Streams"
 *   https://www.cs.rutgers.edu/~muthu/bquant.pdf
 *
 *   Only the biased version is implemented (the targeted version is flawed).
 *
 *   The error is bounded by the rank of the element (not the absolute value).
 *   Specifically, the worst case relative error is bounded by max(eps*p, eps*p0)
 *   where eps is an accuracy paramter, p0 is the `targetProb` and p is the
 *   p-value set as the parameter of function `predict`.
 *
 * @class
 * @param {module:analytics~BiasedGkParam | module:fs:FIn} [arg] - Constructor arguments. There are 2 ways of constructing:
 * <br>1. Using the {@link module:analytics~BiasedGkParam} object,
 * <br>2. using the file input stream {@link module:fs.FIn}.
 *
 * @example
 * // import modules
 * var qm = require('qminer');
 * var analytics = qm.analytics;
 *
 * // create the default TDigest object
 * var gk = new analytics.BiasedGk({
 *     eps: 0.1,
 *     targetProb: 0.99,
 *     compression: 'periodic',
 *     useBands: true
 * });
 *
 * // create the data used for calculating quantiles
 * var inputs = [10, 1, 2, 8, 9, 5, 6, 4, 7, 3];
 *
 * // fit the TDigest model
 * for (var i = 0; i < inputs.length; i++) {
 *     gk.partialFit(inputs[i]);
 * }
 *
 * // make the prediction for the 0.1 quantile
 * var prediction = gk.predict(0.1);
 * // save the model
 * gk.save(fs.openWrite('gk.bin')).close();
 * // open the gk model under a new variable
 * var gk2 = new analytics.Gk(fs.openRead('gk.bin'));
 *
 */
//# exports.BiasedGk = function (arg) { return Object.create(require('qminer').analytics.BiasedGk.prototype); }
class TNodeJsBiasedGk : public node::ObjectWrap {
    friend class TNodeJsUtil;
public:
    static void Init(v8::Handle<v8::Object> exports);
    static const TStr GetClassId() { return "BiasedGk"; }

private:
    TQuant::TBiasedGk Gk;

    TNodeJsBiasedGk(const PJsonVal&);
    TNodeJsBiasedGk(TSIn&);
    ~TNodeJsBiasedGk() {}

    static TNodeJsBiasedGk* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

public:
    /**
     * Returns the models' parameters as a JavaScript object (JSON). These parameters
     * are the same as are set through the constructor.
     *
     * @returns {module:analytics~BiasedGkParam} The construction parameters.
     *
     * var analytics = qm.analytics;
     * var gk = new analytics.BiasedGk();
     * var params = gk.getParams();
     *
     * console.log(params.targetProb);
     * console.log(params.eps);
     * console.log(params.autoCompress);
     * console.log(params.useBands);
     */
    //# exports.BiasedGk.prototype.getParams = function () { return { }; }
    JsDeclareFunction(getParams);

    /**
     * Adds a new value to the summary.
     *
     * @param {number} val - the value
     * @returns {module:analytics.Gk} reference to self
     *
     * @example
     * var qm = require('qminer');
     *
     * var gk = new qm.analytics.BiasedGk();
     * gk.partialFit(1.0);
     * gk.partialFit(2.0);
     */
    //# exports.BiasedGk.compress = function (fout) { return Object.create(require('qminer').analytics.BiasedGk.prototype); }
    JsDeclareFunction(partialFit);

    /**
     * Given an input cumulative probability, returns a quantile associated with that
     * probability (e.g. for input 0.5 it will return the median).
     *
     * @param {number|Array} pVals - the p-values which we a querying
     * @returns {number|Array} quantiles - depending whether the input was a single value or array the method returns a quantile or array of quantiles
     *
     * @example
     * var qm = require('qminer');
     *
     * var gk = new qm.analytics.BiasedGk({
     *     eps: 0.1,
     *     targetProb: 0.01
     * });
     * gk.partialFit(1.0);
     * gk.partialFit(2.0);
     * gk.partialFit(1.0);
     * gk.partialFit(3.0);
     * gk.partialFit(2.0);
     *
     * console.log(gk.predict(0.01));   // prints the first percentile
     * console.log(gk.predict(0.25));   // prints the first quartile
     * console.log(gk.predict(0.5));    // prints the median
     */
    //# exports.BiasedGk.prototype.predict = function (x) { return 0; }
    JsDeclareFunction(predict);

    /**
     * Manually runs the compression procedure.
     *
     * @returns reference to self
     */
    //# exports.BiasedGk.compress = function (fout) { return Object.create(require('qminer').analytics.BiasedGk.prototype); }
    JsDeclareFunction(compress);

    /**
     * Saves the objects state into the output stream.
     *
     * @param {module:fs.FOut} fout - the output stream
     * @returns {module:fs.FOut} - the output stream
     */
    //# exports.BiasedGk.save = function (fout) { return Object.create(require('qminer').fs.FOut.prototype); }
    JsDeclareFunction(save);

private:
    TQuant::TBiasedGk::TCompressStrategy ExtractCompressStrategy(const PJsonVal&);
};


/**
* @typedef {Object} CountWindowGkParam
* An object used for the construction of {@link module:analytics.CountWindowGk}.
* @property {number} [windowSize=10000] - Number of values to store in the window.
* @property {number} [quantileEps=0.01] - Worst-case error of the quantile estimation procedure.
* @property {number} [countEps=0.005] - Worst-case error of the sliding window (exponential histogram) procedure.
*/

/**
 * @classdesc Greenwald - Khanna algorithm for quantile estimation on sliding windows. Given
 *   a cumulative probability p, the algorithm returns the approximate value of the
 *   p-th quantile of all the values in a sliding window.
 *
 *   The algorithm works by keeping a summary of buckets. Each bucket summarizes a
 *   range of values. Through the run of the algorithm, new buckets are created and
 *   old ones merged. To allow for the computation on a sliding window, each bucket
 *   uses an Exponential Histogram structure to remember how many values it summarizes.
 *
 *   It is summarized in:
 *   "Online Algorithm for Approximate Quantile Queries on Sliding Windows"
 *   http://dl.acm.org/citation.cfm?id=2954329
 *
 *   The error is not bounded by the absolute value of the output, but by the error
 *   in rank of the output element. For instance, if we have 100 elements and query
 *   the median, we could get the 48-th (p=0.48), 50-th (p=0.5), 51-th (p=0.51),
 *   etc. element.
 *
 *   The algorithms error is bounded by two factors. The first is the quantile
 *   estimation factor `quantileEps` occurs because of the summary structure and
 *   defines the maximum size of the buckets. The second type of error `countEps`
 *   occurs because of the Exponential Histograms inside the buckets.
 *
 *   The worst-case error is bounded by (quantileEps + 2*countEps + O(countEps^2)),
 *   although in practice the error is lower.
 *
 *   This version of the algorithm uses a count-based fixed size sliding window.
 *
 * @class
 * @param {module:analytics~FixedWindowGkParam | module:fs.FIn} [arg] - Construction arguments. There are two ways of constructing:
 * <br>1. Using the {@link module:analytics~FixedWindowGkParam} object,
 * <br>2. using the file input stream {@link module:fs.FIn}.
 *
 * @example
 * // import modules
 * var qm = require('qminer');
 * var analytics = qm.analytics;
 *
 * // create the default TDigest object
 * var gk = new analytics.CountWindowGk({
 *     windowSize: 5,
 *     quantileEps: 0.001,
 *     countEps: 0.0005
 * });
 *
 * // create the data used for calculating quantiles
 * var inputs = [10, 1, 2, 8, 9, 5, 6, 4, 7, 3];
 *
 * // fit the TDigest model
 * for (var i = 0; i < inputs.length; i++) {
 *     gk.partialFit(inputs[i]);
 * }
 *
 * // make the prediction for the 0.1 quantile
 * var prediction = gk.predict(0.1);
 * // save the model
 * gk.save(fs.openWrite('gk.bin')).close();
 * // open the gk model under a new variable
 * var gk2 = new analytics.CountWindowGk(fs.openRead('gk.bin'));
 *
 */
//# exports.CountWindowGk = function (arg) { return Object.create(require('qminer').analytics.CountWindowGk.prototype); }
class TNodeJsCountWindowGk : public node::ObjectWrap {
    friend class TNodeJsUtil;
public:
    static void Init(v8::Handle<v8::Object> exports);
    static const TStr GetClassId() { return "CountWindowGk"; }

private:
    TQuant::TCountWindowGk Gk;

    TNodeJsCountWindowGk(const PJsonVal& ParamVal);
    TNodeJsCountWindowGk(TSIn& SIn);
    ~TNodeJsCountWindowGk();

    static TNodeJsCountWindowGk* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

public:
    /**
     * Returns the models' parameters as a JavaScript object (JSON). These parameters
     * are the same as are set through the constructor.
     *
     * @returns {module:analytics~FixedWindowGkParam} The construction parameters.
     *
     * var analytics = qm.analytics;
     * var gk = new analytics.CountWindowGk();
     * var params = tdigest.getParams();
     *
     * console.log(params.windowSize);
     * console.log(params.quantileEps);
     * console.log(params.countEps);
     */
    //# exports.CountWindowGk.prototype.getParams = function () { return { }; }
    JsDeclareFunction(getParams);

    /**
     * Appends a new value to the sliding window. If an old value
     * falls outside the sliding window, it is forgotten.
     *
     * @param {number} val - the value
     * @returns {module:analytics.CountWindowGk} reference to self
     *
     * @example
     * var qm = require('qminer');
     *
     * var gk = new qm.analytics.CountWindowGk();
     * gk.partialFit(1.0);
     * gk.partialFit(2.0);
     *
     */
    //# exports.CountWindowGk.partialFit = function (fout) { return Object.create(require('qminer').analytics.CountWindowGk.prototype); }
    JsDeclareFunction(partialFit);

    /**
     * Given an input cumulative probability, returns a quantile associated with that
     * probability (e.g. for input 0.5 it will return the median).
     *
     * @param {number} p - cumulative probability between 0 and 1 (both inclusive)
     * @returns {number} quantile associated with p
     *
     * @example
     * var qm = require('qminer');
     *
     * var gk = new qm.analytics.CountWindowGk({
     *     windowSize: 100    // window 100 elements long
     * });
     * gk.partialFit(1.0);
     * gk.partialFit(2.0);
     * gk.partialFit(1.0);
     * gk.partialFit(3.0);
     * gk.partialFit(2.0);
     *
     * console.log(gk.predict(0.01));   // prints the first percentile
     * console.log(gk.predict(0.25));   // prints the first quartile
     * console.log(gk.predict(0.5));    // prints the median
     */
    //# exports.CountWindowGk.prototype.predict = function (x) { return 0; }
    JsDeclareFunction(predict);

    /**
     * Saves the objects state into a binary file.
     *
     * @param {module:fs.FOut} fout - the output stream
     * @returns {module:fs.FOut} the output stream `fout`
     *
     * @example
     * var gk = new qm.analytics.CountWindowGk();
     *
     * // save the model
     * gk.save(fs.openWrite('tdigest.bin')).close();
     * // open the tdigest model under a new variable
     * var gk = new analytics.CountWindowGk(fs.openRead('tdigest.bin'));
     */
    //# exports.CountWindowGk.save = function (fout) { return Object.create(require('qminer').fs.FOut.prototype); }
    JsDeclareFunction(save);
};

/**
* @typedef {Object} TimeWindowGkParam
* An object used for the construction of {@link module:analytics.TimeWindowGk}.
* @property {number} [window=1000*60*60] - Duration of the time window.
* @property {number} [quantileEps=0.01] - Worst-case error of the quantile estimation procedure.
* @property {number} [countEps=0.005] - Worst-case error of the sliding window (exponential histogram) procedure.
*/

/**
 * @classdesc Greenwald - Khanna algorithm for quantile estimation on sliding windows. Given
 *   a cumulative probability p, the algorithm returns the approximate value of the
 *   p-th quantile of all the values in a sliding window.
 *
 *   The algorithm works by keeping a summary of buckets. Each bucket summarizes a
 *   range of values. Through the run of the algorithm, new buckets are created and
 *   old ones merged. To allow for the computation on a sliding window, each bucket
 *   uses an Exponential Histogram structure to remember how many values it summarizes.
 *
 *   It is summarized in:
 *   "Online Algorithm for Approximate Quantile Queries on Sliding Windows"
 *   http://dl.acm.org/citation.cfm?id=2954329
 *
 *   The error is not bounded by the absolute value of the output, but by the error
 *   in rank of the output element. For instance, if we have 100 elements and query
 *   the median, we could get the 48-th (p=0.48), 50-th (p=0.5), 51-th (p=0.51),
 *   etc. element.
 *
 *   The algorithms error is bounded by two factors. The first is the quantile
 *   estimation factor `quantileEps` occurs because of the summary structure and
 *   defines the maximum size of the buckets. The second type of error `countEps`
 *   occurs because of the Exponential Histograms inside the buckets.
 *
 *   The worst-case error is bounded by (quantileEps + 2*countEps + O(countEps^2)),
 *   although in practice the error is lower.
 *
 *   This version of the algorithm uses a time-based fixed duration sliding window.
 *
 * @class
 * @param {module:analytics~TimeWindowGkParam | module:fs.FIn} [arg] - Construction arguments. There are two ways of constructing:
 * <br>1. Using the {@link module:analytics~TimeWindowGkParam} object,
 * <br>2. using the file input stream {@link module:fs.FIn}.
 *
 * @example
 * // import modules
 * var qm = require('qminer');
 * var analytics = qm.analytics;
 *
 * // create the default TDigest object
 * var gk = new analytics.TimeWindowGk({
 *     window: 5,
 *     quantileEps: 0.001,
 *     countEps: 0.0005
 * });
 *
 * // create the data used for calculating quantiles
 * var inputs = [10, 1, 2, 8, 9, 5, 6, 4, 7, 3];
 * var times = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
 *
 * // fit the TDigest model
 * for (var i = 0; i < inputs.length; i++) {
 *     gk.partialFit(times[i], inputs[i]);
 * }
 *
 * // make the prediction for the 0.1 quantile
 * var prediction = gk.predict(0.1);
 * // save the model
 * gk.save(fs.openWrite('gk.bin')).close();
 * // open the gk model under a new variable
 * var gk2 = new analytics.TimeWindowGk(fs.openRead('gk.bin'));
 */
//# exports.TimeWindowGk = function (arg) { return Object.create(require('qminer').analytics.TimeWindowGk.prototype); }
class TNodeJsTimeWindowGk : public node::ObjectWrap {
    friend class TNodeJsUtil;
public:
    static void Init(v8::Handle<v8::Object> exports);
    static const TStr GetClassId() { return "TimeWindowGk"; }

private:
    TQuant::TTimeWindowGk Gk;

    TNodeJsTimeWindowGk(const PJsonVal& ParamVal);
    TNodeJsTimeWindowGk(TSIn& SIn);
    ~TNodeJsTimeWindowGk();

    static TNodeJsTimeWindowGk* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

public:
    /**
     * Returns the models' parameters as a JavaScript object (JSON). These parameters
     * are the same as are set through the constructor.
     *
     * @returns {module:analytics~TimeWindowGkParam} The construction parameters.
     *
     * var analytics = qm.analytics;
     * var gk = new analytics.TimeWindowGk();
     * var params = tdigest.getParams();
     *
     * console.log(params.window);
     * console.log(params.quantileEps);
     * console.log(params.countEps);
     */
    //# exports.TimeWindowGk.prototype.getParams = function () { return { }; }
    JsDeclareFunction(getParams);

    /**
     * Adds a new observation to the window. The window is updated with the provided timestamp,
     * all records which fall outside the new window are forgotten.
     *
     * @param {number|Date} timestamp - time of the observation
     * @param {number} [value] - the observation
     * @returns {module:analytics.TimeWindowGk} reference to self
     *
     * @example
     * var qm = require('qminer');
     *
     * var gk = new qm.analytics.TimeWindowGk({
     *     window: 1000*60*60*24*2    // window is 2 days
     * });
     * gk.partialFit(new Date('2017-06-06'), 1.0);
     * gk.partialFit(new Date('2017-06-07').getTime(), 1.0);
     * gk.partialFit(new Date('2017-06-08'));  // only move the time window, the first value is forgotten
     */
    //# exports.TimeWindowGk.partialFit = function (fout) { return Object.create(require('qminer').analytics.TimeWindowGk.prototype); }
    JsDeclareFunction(partialFit);

    /**
     * Given an input cumulative probability, returns a quantile associated with that
     * probability (e.g. for input 0.5 it will return the median).
     *
     * @param {number} p - cumulative probability between 0 and 1 (both inclusive)
     * @returns {number} quantile associated with p
     *
     * @example
     * var qm = require('qminer');
     *
     * var gk = new qm.analytics.TimeWindowGk({
     *     window: 100    // window is 2 days
     * });
     * gk.partialFit(0, 1.0);
     * gk.partialFit(1, 1.0);
     * gk.partialFit(2, 1.0);
     * gk.partialFit(3, 1.0);
     * gk.partialFit(4, 1.0);
     *
     * console.log(gk.predict(0.01));   // prints the first percentile
     * console.log(gk.predict(0.25));   // prints the first quartile
     * console.log(gk.predict(0.5));    // prints the median
     */
    //# exports.TimeWindowGk.prototype.predict = function (x) { return 0; }
    JsDeclareFunction(predict);

    /**
     * Saves the objects state into a binary file.
     *
     * @param {module:fs.FOut} fout - the output stream
     * @returns {module:fs.FOut} the output stream `fout`
     *
     * @example
     * var gk = new qm.analytics.TimeWindowGk();
     *
     * // save the model
     * gk.save(fs.openWrite('tdigest.bin')).close();
     * // open the tdigest model under a new variable
     * var gk = new analytics.TimeWindowGk(fs.openRead('tdigest.bin'));
     */
    //# exports.TimeWindowGk.save = function (fout) { return Object.create(require('qminer').fs.FOut.prototype); }
    JsDeclareFunction(save);
};


/////////////////////////////////////////////
// QMiner-JavaScript-Recommender System

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
//# exports.RecommenderSys = function (arg) { return Object.create(require('qminer').analytics.RecommenderSys.prototype); }
class TNodeJsRecommenderSys : public node::ObjectWrap {
    friend class TNodeJsUtil;
public:
    static void Init(v8::Handle<v8::Object> exports);
    static const TStr GetClassId() { return "RecommenderSys"; }
    ~TNodeJsRecommenderSys() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; }

private:
    int Iter;
    int K;
    double Tol;
    bool Verbose;
    PNotify Notify;

    TFltVV U;
    TFltVV V;

    TNodeJsRecommenderSys(const PJsonVal& ParamVal);
    TNodeJsRecommenderSys(TSIn& SIn);

    static TNodeJsRecommenderSys* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

private:
    class TFitTask : public TNodeTask {
        TNodeJsRecommenderSys* JsRecSys;
        TNodeJsFltVV*  JsFltVV;
        TNodeJsSpMat*  JsSpVV;

    public:
        TFitTask(const v8::FunctionCallbackInfo<v8::Value>& Args);

        v8::Handle<v8::Function> GetCallback(const v8::FunctionCallbackInfo<v8::Value>& Args);
        void Run();
    };

public:

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
    //# exports.RecommenderSys.prototype.getParams = function () { return { iter: 10000, k: 2, tol: 1e-3, verbose: false }; }
    JsDeclareFunction(getParams);

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
    //# exports.RecommenderSys.prototype.setParams = function (params) { return Object.create(require('qminer').analytics.RecommenderSys.prototype); }
    JsDeclareFunction(setParams);

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
    //# exports.RecommenderSys.prototype.getModel = function () { return { U: Object.create(require('qminer').la.Matrix.prototype), V: Object.create(require('qminer').la.Matrix.prototype) }; }
    JsDeclareFunction(getModel);

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
    //# exports.RecommenderSys.prototype.fit = function (A) { return Object.create(require('qminer').analytics.RecommenderSys.prototype); }
    JsDeclareSyncAsync(fit, fitAsync, TFitTask);

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
    //# exports.RecommenderSys.prototype.save = function (fout) { return Object.create(require('qminer').fs.FOut.prototype); }
    JsDeclareFunction(save);

private:
    void UpdateParams(const PJsonVal& ParamVal);
    PJsonVal GetParams() const;

    void Save(TSOut& SOut) const;
};

/////////////////////////////////////////////
// QMiner-JavaScript-Graph-Cascade

//# exports.GraphCascade = function (arg) { return Object.create(require('qminer').analytics.GraphCascade.prototype); }
class TNodeJsGraphCascade : public node::ObjectWrap {
    friend class TNodeJsUtil;
public:
    static void Init(v8::Handle<v8::Object> exports);
    static const TStr GetClassId() { return "GraphCascade"; }
    ~TNodeJsGraphCascade() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; }

private:
    TGraphProcess::TGraphCascade Model;

private:

    TNodeJsGraphCascade(const PJsonVal& ParamVal) : Model(ParamVal) {}

    static TNodeJsGraphCascade* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

public:
    /**
    * Sets the cascade time for a given node
    * @param {string} nodeId -
    * @param {number} timestamp -
    */
    JsDeclareFunction(observeNode);

    /**
    * Computes the posterior for timestamps of unobserved nodes
    * @param {number} timestamp - current time
    */
    JsDeclareFunction(computePosterior);
    /**
    * Returns the posteriors
    * @returns {Object} - model
    */
    JsDeclareFunction(getPosterior);
    /**
    * Returns the pruned directed acyclic graph
    * @returns {Object} - dag
    */
    JsDeclareFunction(getGraph);
    /**
    * Returns the topologically ordered node names
    * @returns {Object} - nodeArr
    */
    JsDeclareFunction(getOrder);

};

#endif /* ANALYTICS_H_ */


#ifndef ANALYTICS_H_
#define ANALYTICS_H_

//#ifndef BUILDING_NODE_EXTENSION
//	#define BUILDING_NODE_EXTENSION
//#endif

#include <node.h>
#include <node_object_wrap.h>
#include "../nodeutil.h"
#include "fs_nodejs.h"
#include "la_nodejs.h"
#include "qminer_ftr.h"
#include "mc.h"

///////////////////////////////
// QMiner-JavaScript-Support-Vector-Machine-Model
// Holds SVM classification or regression model. 
// TODO rewrite to JavaScript
class TNodeJsSvmModel : public node::ObjectWrap {
	friend class TNodeJsUtil;
	friend class TNodeJsSVC;
	friend class TNodeJsSVR;
private:
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

	TSvm::TLinModel* Model;

	TNodeJsSvmModel(const PJsonVal& ParamVal);
	TNodeJsSvmModel(TSIn& SIn);
	~TNodeJsSvmModel();

	static v8::Local<v8::Object> WrapInst(v8::Local<v8::Object> Obj, const PJsonVal& ParamVal);
	static v8::Local<v8::Object> WrapInst(v8::Local<v8::Object> Obj, TSIn& SIn);

public:
	JsDeclareFunction(New);
	//
	// **Functions and properties:**
	//
	//- `params = svmModel.getParams()` -- returns the parameters of this model as a Javascript object
	JsDeclareFunction(getParams);
	//- `svmModel = svmModel.getParams(params)` -- sets one or more parameters given in the input argument `params` returns this
	JsDeclareFunction(setParams);
    //- `vec = svmModel.weights` -- weights of the SVM linear model as a full vector `vec`
	JsDeclareProperty(weights);
    //- `fout = svmModel.save(fout)` -- saves model to output stream `fout`. Returns `fout`.
	JsDeclareFunction(save);
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
//#
//# ### Support Vector Classification
//#
//# Holds a SVM classification model. This object is result of `new analytics.SVC(...)`.
class TNodeJsSVC : public TNodeJsSvmModel {
	static v8::Persistent <v8::Function> constructor;
public:
	static void Init(v8::Handle<v8::Object> exports);
	//#
	//# **Constructor:**
	//#
	//#- `svmModel = new analytics.SVC(fin)` -- constructs a new support vector classifier
	//#- `svmModel = new analytics.SVC(svmParameters)` -- constructs a new support vector classifier using `svmParameters`, which is a JSON object. `svmParameters = {c: 1.0, j: 1.0, batchSize: 10000, maxIterations: 10000, maxTime: 600, minDiff: 1e-6, verbose: false}`. 
	//#     The parameter `c` is the SVM cost parameter, `j` (factor to multiply SVM cost parameter for positive examples with (default is 1.0)), `batchSize` controls the sample size for stochastic subgradient calculations, `maxIterations` limits the number of subgradient steps, `maxTime` limits the runtime in seconds, `minDiff` is a tolerance that is used as a stopping condition, `verbose` controls verbosity of the algorithm; result is a linear model
	//#
	//# **Functions and properties:**
	//#
	//#- `params = SVC.getParams()` -- returns the parameters of this model as a Javascript object
	//#- `svmModel = SVC.getParams(params)` -- sets one or more parameters given in the input argument `params` returns this
	//#- `vec = SVC.weights` -- weights of the SVM linear model as a full vector `vec`
	//#- `fout = SVC.save(fout)` -- saves model to output stream `fout`. Returns `fout`.
	//#- `num = SVC.predict(vec)` -- sends vector `vec` through the model and returns the prediction as a real number `num` (-1 or 1 for classification)
	//#- `num = SVC.predict(spVec)` -- sends sparse vector `spVec` through the model and returns the prediction as a real number `num` (-1 or 1 for classification)
	//#- `svmModel = SVC.fit(spMat,vec)` -- fits an SVM model, given column examples in a sparse matrix `spMat` and vector of targets `vec`
	//#- `svmModel = SVC.fit(mat,vec)` -- fits an SVM model, given column examples in a matrix `mat` and vector of targets `vec`
	JsDeclareFunction(fit);
};

///////////////////////////////
// QMiner-JavaScript-Support-Vector-Regression
//#
//# ### Support Vector Regression
//#
//# Holds a SVM regression model. This object is result of `new analytics.SVR(...)`.
class TNodeJsSVR : public TNodeJsSvmModel {
	static v8::Persistent <v8::Function> constructor;
public:
	static void Init(v8::Handle<v8::Object> exports);
	//#
	//# **Constructor:**
	//#
	//#- `svmModel = new analytics.SVR(fin)` -- constructs a new support vector classifier
	//#- `svmModel = new analytics.SVR(svmParameters)` -- constructs a new support vector regression using `svmParameters`, which is a JSON object. `svmParameters = {c: 1.0, j: 1.0, batchSize: 10000, maxIterations: 10000, maxTime: 600, minDiff: 1e-6, verbose: false}`. 
	//#     The parameter `c` is the SVM cost parameter, `j` (factor to multiply SVM cost parameter for positive examples with (default is 1.0)), `batchSize` controls the sample size for stochastic subgradient calculations, `maxIterations` limits the number of subgradient steps, `maxTime` limits the runtime in seconds, `minDiff` is a tolerance that is used as a stopping condition, `verbose` controls verbosity of the algorithm; result is a linear model
	//#
	//# **Functions and properties:**
	//#
	//#- `params = SVR.getParams()` -- returns the parameters of this model as a Javascript object
	//#- `svmModel = SVR.getParams(params)` -- sets one or more parameters given in the input argument `params` returns this
	//#- `vec = SVR.weights` -- weights of the SVM linear model as a full vector `vec`
	//#- `fout = SVR.save(fout)` -- saves model to output stream `fout`. Returns `fout`.
	//#- `svmModel = SVR.fit(spMat,vec)` -- fits an SVM model, given column examples in a sparse matrix `spMat` and vector of targets `vec`
	//#- `svmModel = SVR.fit(mat,vec)` -- fits an SVM model, given column examples in a matrix `mat` and vector of targets `vec`
	//#- `num = SVR.predict(vec)` -- sends vector `vec` through the model and returns the prediction as a real number `num` (-1 or 1 for classification)
	//#- `num = SVR.predict(spVec)` -- sends sparse vector `spVec` through the model and returns the prediction as a real number `num` (-1 or 1 for classification)
	JsDeclareFunction(fit);	
};

///////////////////////////////
// QMiner-JavaScript-Recursive-Linear-Regression
//#
//# ### Recursive Linear Regression model
//#
//# Holds online regression model.
class TNodeJsRecLinReg : public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	TSignalProc::PRecLinReg Model;
	TNodeJsRecLinReg(const TSignalProc::PRecLinReg& Model);
public:
	static void Init(v8::Handle<v8::Object> exports);
	static const TStr ClassId;
private:
	//#
	//# **Constructor:**
	//#
	//#- `recLinRegModel = new analytics.RecLinReg(fin)` -- constructs a recursive linear regression model by loading it from input stream `fin`
	//#- `recLinRegModel = new analytics.RecLinReg(recLinRegParameters)` -- constructs a recursive linear regression using a JSON parameter object `recLinRegParameters, whose properties are `recLinRegParameters.dim` (dimensionality of feature space, e.g.
	//#     `ftrSpace.dim`), `recLinRegParameters.forgetFact` (forgetting factor, default is 1.0) and `recLinRegParameters.regFact` 
	//#     (regularization parameter to avoid over-fitting, default is 1.0).)
	static TNodeJsRecLinReg* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);
	//#
	//# **Functions and properties:**
	//#
    //#- `recLinRegModel = recLinRegModel.fit(vec, num)` -- updates the model using full vector `vec` and target number `num`as training data. Returns self.
	JsDeclareFunction(fit);
    //#- `num = recLinRegModel.predict(vec)` -- sends vector `vec` through the
    //#     model and returns the prediction as a real number `num`
	JsDeclareFunction(predict);

	//#- `params = svmModel.getParams()` -- returns the parameters of this model as
	//#- a Javascript object
	JsDeclareFunction(getParams);

    //#- `vec = recLinRegModel.weights` -- weights of the linear model as a full vector `vec`
	JsDeclareProperty(weights);
    //#- `num = recLinRegModel.dim` -- dimensionality of the feature space on which this model works
	JsDeclareProperty(dim);
	//#- `fout = recLinRegModel.save(fout)` -- saves model to output stream `fout`. Returns `fout`.
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
	static const TStr ClassId;	// set to LogReg
	static void Init(v8::Handle<v8::Object> exports);

private:
	TMl::TLogReg LogReg;

	TNodeJsLogReg(const TMl::TLogReg& _LogReg): LogReg(_LogReg) {}

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
// Exponential Regression
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
class TNodeJsExpReg : public node::ObjectWrap {
	friend class TNodeJsUtil;
public:
	static const TStr ClassId;
	static void Init(v8::Handle<v8::Object> exports);

private:
	TMl::TExpReg ExpReg;

	TNodeJsExpReg(const TMl::TExpReg& _ExpReg): ExpReg(_ExpReg) {}

	static TNodeJsExpReg* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

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
class TNodeJsHMChain : public node::ObjectWrap, public TMc::TMcCallback {
	friend class TNodeJsUtil;
public:
	static const TStr ClassId;
	static void Init(v8::Handle<v8::Object> exports);

private:
	const static double DEFAULT_DELTA_TM;

	TMc::PHierarchCtmc McModel;

	v8::Persistent<v8::Function> StateChangedCallback;
	v8::Persistent<v8::Function> AnomalyCallback;
	v8::Persistent<v8::Function> OutlierCallback;

	TNodeJsHMChain(const TMc::PHierarchCtmc& McModel);
	TNodeJsHMChain(PSIn& SIn);

	~TNodeJsHMChain();

	static TNodeJsHMChain* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

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
	//#- `hmc.update(ftrVec, recTm)` TODO write documentation
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
	 * Returns the centroid of the specified state.
	 *
	 * @param {Number} stateId - the ID of the state
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
	 * Rebuilds its hierarchy.
	 */
	JsDeclareFunction(rebuildHierarchy);

	/**
	 * Rebuilds the histograms using the instances stored in the columns of X.
	 *
	 * @param {Matrix} X - the column matrix containing data instances
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

	// parameters
	//#- `hmc = hmc.getParams(params)` -- sets one or more parameters given
	//#- in the input argument `params` returns this
	JsDeclareFunction(setParams);

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

private:
	void SetParams(const PJsonVal& ParamVal);
	void InitCallbacks();
};

///////////////////////////////
// QMiner-JavaScript-Neural-Networks
//#
//# ### Neural Network model
//#
//# Holds online/offline neural network model. This object is result of `analytics.newNNet`.
class TNodeJsNNet : public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	TSignalProc::PNNet Model;

	TNodeJsNNet(const PJsonVal& ParamVal);
	TNodeJsNNet(TSIn& SIn);
	static TNodeJsNNet* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

public:
	static const TStr ClassId;
	static void Init(v8::Handle<v8::Object> exports);

    //#- `NNet = NNet.fit(vec,vec)` -- fits the NNet model in online mode
    //#- `NNet = NNet.fit(mat,mat)` -- fits the NNet model in batch mode
	JsDeclareFunction(fit);
    //#- `vec = NNet.predict(vec)` -- sends vector `vec` through the
    //#     model and returns the prediction as a vector `vec`
	JsDeclareFunction(predict);
	//#- `NNet.setLearnRate(num)` -- Sets the new learn rate for the network
	JsDeclareFunction(setLearnRate);
	//#- `NNet.save(fout)` -- Saves the model into the specified output stream.
	JsDeclareFunction(save);
 private:
	TSignalProc::TTFunc ExtractFuncFromString(const TStr& FuncString);
};
// QMiner-JavaScript-Tokenizer
//#
//# ### Tokenizer
//#
//# Breaks text into tokens (i.e. words).
class TNodeJsTokenizer : public node::ObjectWrap {
public:
	/// Tokenizer Model
	PTokenizer Tokenizer;
	static v8::Persistent <v8::Function> constructor;
private:
	TNodeJsTokenizer(const PTokenizer& _Tokenizer): 
		Tokenizer(_Tokenizer) { }
public:
	static void Init(v8::Handle<v8::Object> exports);
	static v8::Local<v8::Object> New(const PTokenizer& Tokenizer);
	//#
	//# **Constructor:**
	//#
	//#- `tokenizer = new analytics.Tokenizer({ type: <type>, ...})` -- create new tokenizer
	//#     of type `<type>`. Syntax same as when defining index keys in stores or `text` feature 
	//#     extractors.
	JsDeclareFunction(New);
	//#
	//# **Functions and properties:**
	//#
	//#- `arr = tokenizer.getTokens(string)` -- tokenizes given strings and returns it as an array of strings.
	JsDeclareFunction(getTokens);
	//#- `arr = tokenizer.getSentences(string)` -- breaks text into sentence and returns them as an array of strings.
	JsDeclareFunction(getSentences);
	//#- `arr = tokenizer.getParagraphs(string)` -- breaks text into paragraphs and returns them as an array of strings.
	JsDeclareFunction(getParagraphs);
};

#endif /* ANALYTICS_H_ */


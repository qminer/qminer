#ifndef ANALYTICS_H_
#define ANALYTICS_H_

#ifndef BUILDING_NODE_EXTENSION
	#define BUILDING_NODE_EXTENSION
#endif

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
//# Holds online regression model. This object is result of `analytics.newRecLinReg`.
class TNodeJsRecLinReg : public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	static v8::Persistent <v8::Function> constructor;

	TSignalProc::PRecLinReg Model;

	TNodeJsRecLinReg(const TSignalProc::PRecLinReg& Model);

	static v8::Local<v8::Object> WrapInst(const v8::Local<v8::Object> Obj, const TSignalProc::PRecLinReg& Model);

public:
//	static v8::Local<v8::Object> New(const TSignalProc::PRecLinReg& Model);

	static void Init(v8::Handle<v8::Object> exports);
	//#
	//# **Constructor:**
	//#
	//#- `recLinRegModel = new analytics.RecLinReg(fin)` -- constructs a recursive linear regression model by loading it from input stream `fin`
	//#- `recLinRegModel = new analytics.RecLinReg(recLinRegParameters)` -- constructs a recursive linear regression using a JSON parameter object `recLinRegParameters, whose properties are `recLinRegParameters.dim` (dimensionality of feature space, e.g.
    //#     `ftrSpace.dim`), `recLinRegParameters.forgetFact` (forgetting factor, default is 1.0) and `recLinRegParameters.regFact` 
    //#     (regularization parameter to avoid over-fitting, default is 1.0).)
	JsDeclareFunction(New);
	//#
	//# **Functions and properties:**
	//#
    //#- `recLinRegModel = recLinRegModel.learn(vec, num)` -- updates the model using full vector `vec` and target number `num`as training data. Returns self.
	JsDeclareFunction(learn);
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

////////////////////////////////////////////////////////
// Hierarchical Markov Chain model
class TNodeJsHMChain : public node::ObjectWrap, public TMc::TMcCallback {
	friend class TNodeJsUtil;
private:
	const static double DEFAULT_DELTA_TM;

	static v8::Persistent <v8::Function> constructor;

	TMc::PHierarchCtmc McModel;

	v8::Persistent<v8::Function> StateChangedCallback;
	v8::Persistent<v8::Function> AnomalyCallback;
	v8::Persistent<v8::Function> OutlierCallback;

	TNodeJsHMChain(const TMc::PHierarchCtmc& McModel);
	TNodeJsHMChain(PSIn& SIn);

	~TNodeJsHMChain();

	static v8::Local<v8::Object> WrapInst(const v8::Local<v8::Object> Obj, const PJsonVal& ParamVal);
	static v8::Local<v8::Object> WrapInst(const v8::Local<v8::Object> Obj, PSIn& SIn);

public:
	static void Init(v8::Handle<v8::Object> exports);
	//#
	//# **Constructor:**
	//#
	//#- `hmc = new analytics.HMC(params)` -- Creates a new model using `params` JSON. TODO param description.
	//#- `hmc = new analytics.HMC(fin)` -- Loads the model from input stream `fin`.
	JsDeclareFunction(New);
	//#
	//# **Functions and properties:**
	//#
	//#- `hmc.fit(ftrColMat, timeV)` -- Initializes the model with the instances in the columns of colMat
	//#- which are sampled at time in timeV.
	JsDeclareFunction(fit);
	//#- `hmc.update(ftrVec, recTm)`
	JsDeclareFunction(update);

	// predictions
	//#- `probs = hmc.futureStates(level, startState[, time])` -- returns a vector of probabilities
	//#- of future states starting from `startState` in time `time`.
	//#- If time is not specified it returns the most likely next states.
	JsDeclareFunction(futureStates);
	//#- `probs = hmc.pastStates(level, startState[, time])` -- returns a vector of probabilities
	//#- of past states starting from `startState` in time `time`.
	//#- If time is not specified it returns the most likely previous states.
	JsDeclareFunction(pastStates);
	//#- `probs = hmc.probsOverTime(level, state, dt)` --
	JsDeclareFunction(probsOverTime);
	//#- `stateIdV = hmc.getPastStates(level)` -- returns the previous states
	JsDeclareFunction(histStates);

	// state
	//#- `hmc.toJSON()` -- Returns a JSON representation of the model
	JsDeclareFunction(toJSON);
	//#- `transitionMat = hmc.getTransitionModel()` -- returns the transition matrix on level 0
	JsDeclareFunction(getTransitionModel);
	//#- `currStateV = hmc.getCurrState([height])` -- returns the current states through the hierarchy, if the height is specified it returns the ID of the current state on that height
	JsDeclareFunction(currState);
	//#- `coords = hmc.fullCoords(stateId)` -- returns the coordinates of the state
	JsDeclareFunction(fullCoords);
	//#- `hist = hmc.histogram(stateId, ftrId)` -- returns the histogram of the specified feature in the specified state
	JsDeclareFunction(histogram);

	// callbacks
	//#- `hmc.onStateChanged(function (stateV) {})` -- callback when the current state changes
	JsDeclareFunction(onStateChanged);
	//#- `hmc.onAnomaly(function (description) {})` -- callback when an anomaly is detected
	JsDeclareFunction(onAnomaly);
	//#- `hmc.onOutlier(function (ftrVec) {})` -- callback when an anomaly is detected
	JsDeclareFunction(onOutlier);

	// rebuild methods
	//#- `hmc.rebuildHierarchy()` -- rebuilds the hierarchy
	JsDeclareFunction(rebuildHierarchy);
	//#- `hmc.rebuildHistograms(ftrColMat)` -- rebuilds the state histograms using the instances stored
	//#- in the columns of the provided matrix
	JsDeclareFunction(rebuildHistograms);

	//#- `name = hmc.setStateName(stateId, stateNm)` -- Returns the name of the specified state.
	JsDeclareFunction(getStateName);
	//#- `hmc.setStateName(stateId, stateNm)` -- Sets the name of the state with the specified ID.
	JsDeclareFunction(setStateName);

	// parameters
	//#- `hmc = hmc.getParams(params)` -- sets one or more parameters given
	//#- in the input argument `params` returns this
	JsDeclareFunction(setParams);

	//#- `hmc.save(fout)` -- Saves the model into the specified output stream.
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
	static v8::Persistent <v8::Function> constructor;
	TSignalProc::PNNet Model;

	TNodeJsNNet(const PJsonVal& ParamVal);
	TNodeJsNNet(TSIn& SIn);

	static v8::Local<v8::Object> WrapInst(v8::Local<v8::Object> Obj, const PJsonVal& ParamVal);
	static v8::Local<v8::Object> WrapInst(v8::Local<v8::Object> Obj, TSIn& SIn);

public:
	static void Init(v8::Handle<v8::Object> exports);

	JsDeclareFunction(New);
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


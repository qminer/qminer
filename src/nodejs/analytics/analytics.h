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
//#
//# ### Support Vector Machine model
//#
//# Holds SVM classification or regression model. This object is result of
//# `analytics.trainSvmClassify` or `analytics.trainSvmRegression`.
// TODO rewrite to JavaScript
class TNodeJsSvmModel : public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	static v8::Persistent <v8::Function> constructor;

	TStr Algorithm;
	double SvmCost;
	double SvmUnbalance;
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
	static void Init(v8::Handle<v8::Object> exports);

	JsDeclareFunction(New);

	//#
	//# **Functions and properties:**
	//#
	//#- `svmModel = svmModel.fit(X,y)` -- fits an SVM model
	JsDeclareFunction(fit);
    //#- `num = svmModel.predict(vec)` -- sends vector `vec` through the model and returns the prediction as a real number `num` (-1 or 1 for classification)
	//#- `num = svmModel.predict(spVec)` -- sends sparse vector `spVec` through the model and returns the prediction as a real number `num` (-1 or 1 for classification)
	JsDeclareFunction(predict);

	//#- `params = svmModel.getParams()` -- returns the parameters of this model as
	//#- a Javascript object
	JsDeclareFunction(getParams);
	//#- `svmModel = svmModel.getParams(params)` -- sets one or more parameters given
	//#- in the input argument `params` returns this
	JsDeclareFunction(setParams);

    //#- `vec = svmModel.weights` -- weights of the SVM linear model as a full vector `vec`
	JsDeclareProperty(weights);
    //#- `fout = svmModel.save(fout)` -- saves model to output stream `fout`. Returns `fout`.
	JsDeclareFunction(save);

private:
	void UpdateParams(const PJsonVal& ParamVal);
	PJsonVal GetParams() const;
	void Save(TSOut& SOut) const;
	void ClrModel();
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

	//#- `hmc = new analytics.HMarkovChain(config, ftrSpace)` -- Creates a new model.
	//#- `hmc = new analytics.HMarkovChain(base, fname)` -- Loads the model from file `fname`.
	//#- `hmc = new analytics.HMarkovChain(base, fin)` -- Loads the model from input stream `fin`.
	JsDeclareFunction(New);

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
	//# **Functions and properties:**
	//#
	JsDeclareFunction(New);
	//#- `arr = tokenizer.getTokens(string)` -- tokenizes given strings and returns it as an array of strings.
	JsDeclareFunction(getTokens);
	//#- `arr = tokenizer.getSentences(string)` -- breaks text into sentence and returns them as an array of strings.
	JsDeclareFunction(getSentences);
	//#- `arr = tokenizer.getParagraphs(string)` -- breaks text into paragraphs and returns them as an array of strings.
	JsDeclareFunction(getParagraphs);
};

#endif /* ANALYTICS_H_ */


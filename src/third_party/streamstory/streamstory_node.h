/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef STREAMSTORY_NODE_H_
#define STREAMSTORY_NODE_H_

#include "../nodeutil.h"
#include "la_nodejs.h"

#include "streamstory.h"

////////////////////////////////////////////////////////
// Hierarchical Markov Chain model
//!
//! **Constructor:**
//!
//!- `hmc = new analytics.HMC(params)` -- Creates a new model using `params` JSON. TODO param description.
//!- `hmc = new analytics.HMC(fin)` -- Loads the model from input stream `fin`.
class TNodeJsStreamStory : public node::ObjectWrap, public TMc::TStreamStoryCallback {
	friend class TNodeJsUtil;
public:
	static void Init(v8::Handle<v8::Object> exports);
	static const TStr GetClassId() { return "_StreamStory"; }

private:
	const static double DEFAULT_DELTA_TM;

	TMc::TStreamStory* StreamStory;

	TMainThreadHandle* UvHandle;

	v8::Persistent<v8::Function> StateChangedCallback;
	v8::Persistent<v8::Function> AnomalyCallback;
	v8::Persistent<v8::Function> OutlierCallback;
	v8::Persistent<v8::Function> ProgressCallback;
	v8::Persistent<v8::Function> PredictionCallback;
	v8::Persistent<v8::Function> ActivityCallback;

	TCriticalSection ProgressSection;
	TVec<TIntStrPr> ProgressQ;

	TNodeJsStreamStory(TMc::TStreamStory* McModel);
	TNodeJsStreamStory(PSIn& SIn);

	~TNodeJsStreamStory();

	static TNodeJsStreamStory* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

private:
	class TFitTask: public TNodeTask {
	private:
		TNodeJsStreamStory* JsStreamStory;
		TNodeJsFltVV* JsObservFtrVV;
		TNodeJsFltVV* JsControlFtrVV;
		TNodeJsFltVV* JsIgnoredFtrVV;
		TNodeJsFltV* JsRecTmV;
		TNodeJsBoolV* JsBatchEndJsV;

		TMc::TFtrInfoV ObsFtrInfo;
		TMc::TFtrInfoV ContrFtrInfo;
		TMc::TFtrInfoV IgnFtrInfo;

	public:
		TFitTask(const v8::FunctionCallbackInfo<v8::Value>& Args);

		v8::Handle<v8::Function> GetCallback(const v8::FunctionCallbackInfo<v8::Value>& Args);
		void Run();
	};

	class TProgressTask: public TMainThreadTask {
	private:
		TNodeJsStreamStory* JsStreamStory;
	public:
		TProgressTask(TNodeJsStreamStory* JsStreamStory);
		void Run();
	};

public:

	JsDeclareSyncAsync(fit,fitAsync,TFitTask);

	//!- `hmc.update(obsFtrV, contrFtrV, recTm)` TODO write documentation
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

	JsDeclareFunction(predictNextState);

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

	JsDeclareFunction(getHistoricalStates);

	/**
	 * Returns an object representation of this model.
	 *
	 * @returns {Object}
	 */
	JsDeclareFunction(toJSON);

	/**
	 * Returns an object representation of a subset of this model.
	 *
	 * @param {Number} stateId - state to zoom into
	 * @returns {Object}
	 */
	JsDeclareFunction(getSubModelJson);

	/**
	 * Returns an object representation of a subset of this model.
	 *
	 * @param {Number} stateId - the starting state id
	 * @param {Number} height - the height on which the path is requested
	 * @param {Number} length - the length of the path
	 * @param {Number} [transTreshold=0.2] - only consider transitions above this threshold
	 * @returns {Object}
	 */
	JsDeclareFunction(getStatePath);

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
	 * @param {Integer} [ftrSpaceN] - indicates wether to output observation or control coordinates
	 * @returns {Array} - the coordinates of the state
	 */
	JsDeclareFunction(fullCoords);

	JsDeclareFunction(getStateCentroids);

	/**
	 * Returns a histogram of the specified feature in the specified state.
	 *
	 * @param {Number} ftrId - the ID of the feature
	 * @param {Number} [stateId] - the ID of the statem if no state is provided, a histogram for all the states is returned
	 * @returns {Array} - the histogram
	 */
	JsDeclareFunction(histogram);

	JsDeclareFunction(transitionHistogram);

	JsDeclareFunction(timeHistogram);

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
	JsDeclareFunction(getWeights);

	/**
	 * Returns a JSON representation of a decision tree, which classifies
	 * this state against other states
	 *
	 * @param {Number} stateId
	 * @returns {Object}
	 */
	JsDeclareFunction(getClassifyTree);

	JsDeclareFunction(explainState);

	// activities
	JsDeclareFunction(setActivity);
	JsDeclareFunction(removeActivity);
	JsDeclareFunction(getActivities);

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

	JsDeclareFunction(onProgress);

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

	JsDeclareFunction(onActivity);

	/**
	 * Rebuilds the histograms using the instances stored in the columns of X.
	 *
	 * @param {Matrix} obsMat - the column matrix containing observation data instances
	 * @param {Matrix} controlMat - the column matrix containing control data instances
	 */
	JsDeclareFunction(rebuildHistograms);

	JsDeclareFunction(getStateLabel);
	JsDeclareFunction(getStateAutoName);
	JsDeclareFunction(narrateState);
	JsDeclareFunction(getStateTypTimes);

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

	JsDeclareFunction(setStateCoords);

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
	 * @returns {Boolean}
	 */
	JsDeclareFunction(isTarget);

	/**
	 * Sets whether the specified state is a target state or not.
	 *
	 * @param {Number} stateId - ID of the state
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

	JsDeclareFunction(isActivityDetector);
	JsDeclareFunction(isPredictor);

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
	void OnStateChanged(const uint64 Tm, const TIntFltPrV& StateIdHeightV);
	void OnAnomaly(const TStr& AnomalyDesc);
	void OnOutlier(const TFltV& FtrV);
	void OnProgress(const int& Perc, const TStr& Msg);
	void OnPrediction(const uint64& RecTm, const int& CurrStateId, const int& TargetStateId,
			const double& Prob, const TFltV& ProbV, const TFltV& TmV);
	void OnActivityDetected(const uint64& StartTm, const uint64& EndTm, const TStr& ActNm);

private:
	void ProcessProgressQ();

	void SetParams(const PJsonVal& ParamVal);
	void InitCallbacks();

	static v8::Local<v8::Object> WrapHistogram(const TFltV& BinValV,
			const TFltV& SourceProbV, const TFltV& TargetProbV, const TFltV& AllProbV);
	static uint64 GetTmUnit(const TStr& TmUnitStr);
	static TClustering::TAbsKMeans<TFltVV>* GetClust(const PJsonVal& ParamJson, const TRnd& Rnd);

	// feature information
	static void ParseFtrInfo(const PJsonVal& InfoJson, TMc::TFtrInfoV& ObsFtrInfoV,
			TMc::TFtrInfoV& ContrFtrInfoV, TMc::TFtrInfoV& IgnFtrInfo);
	static void ParseFtrInfo(const PJsonVal& InfoJson, TMc::TFtrInfoV& FtrInfoV);
};

#endif

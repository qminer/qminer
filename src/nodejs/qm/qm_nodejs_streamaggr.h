/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef QMINER_QM_NODEJS_STREAMAGGR
#define QMINER_QM_NODEJS_STREAMAGGR

#include <node.h>
#include <node_object_wrap.h>
#include <qminer.h>
#include "../la/la_nodejs.h"
#include "../fs/fs_nodejs.h"
#include "../nodeutil.h"

/**
* QMiner module.
* @module qm
* @example
* // import module
* var qm = require('qminer');
*/

/**
* Stream Aggregate
* @classdesc Represents a stream aggregate. The class can construct these {@link module:qm~StreamAggregator} objects. Also turn to these stream aggregators to see
* which methods are implemented.
* @class
* @param {module:qm.Base} base - The base object on which it's created.
* @param {(module:qm~StreamAggregator | function)} arg - Constructor arguments. There are two argument types:
* <br>1. Using the {@link module:qm~StreamAggregator} object,
* <br>2. using a function/JavaScript class. The function has defined The object containing the schema of the stream aggregate or the function object defining the operations of the stream aggregate.
* @param {(string | Array.<string>)} [storeName] - The store names where the aggregate will be registered.
* @example
* // import qm module
* var qm = require('qminer');
* // create a simple base containing one store
* var base = new qm.Base({
*    mode: "createClean",
*    schema: [{
*        name: "People",
*        fields: [
*            { name: "Name", type: "string" },
*            { name: "Gendre", type: "string" },
*        ]
*    },
*    {
*        name: "Laser",
*        fields: [
*            { name: "Time", type: "datetime" },
*            { name: "WaveLength", type: "float" }
*        ]
*    }]
* });
*
* // create a new stream aggregator for 'People' store, get the length of the record name (with the function object)
* var aggr = new qm.StreamAggr(base, new function () {
*    var numOfAdds = 0;
*    var numOfUpdates = 0;
*    var numOfDeletes = 0;
*    var time = "";
*    this.name = 'nameLength',
*    this.onAdd = function (rec) {
*        numOfAdds += 1;
*    };
*    this.onUpdate = function (rec) {
*        numOfUpdates += 1;
*    };
*    this.onDelete = function (rec) {
*        numOfDeletes += 1;
*    };
*    this.onTime = function (ts) {
*        time = ts;
*    };
*    this.saveJson = function (limit) {
*        return { adds: numOfAdds, updates: numOfUpdates, deletes: numOfDeletes, time: time };
*    };
* }, "People");
*
* // create a new time series window buffer stream aggregator for 'Laser' store (with the JSON object)
* var wavelength = {
*     name: "WaveLengthLaser",
*     type: "timeSeriesWinBuf",
*     store: "Laser",
*     timestamp: "Time",
*     value: "WaveLength",
*     winsize: 10000
* }
* var sa = base.store("Laser").addStreamAggr(wavelength);
* base.close();
*/
//# exports.StreamAggr = function (base, json, storeName) { return Object.create(require('qminer').StreamAggr.prototype); };

/**
* @typedef {module:qm.StreamAggr} StreamAggregator
* Stream aggregator types.
* @property {module:qm~StreamAggrTimeSeriesWindow} timeSeries - The time series type.
* @property {module:qm~StreamAggrTimeSeriesWindowVector} timeSeriesBufferVector - The time series buffer vector type.
* @property {module:qm~StreamAggrTimeSeriesTick} tick - The time series tick type.
* @property {module:qm~StreamAggrRecordBuffer} record-buffer - The record buffer type.
* @property {module:qm~StreamAggrFeatureSpace} ftr-space - The feature space type.
* @property {module:qm~StreamAggrSum} sum - The sum type. Calculates the sum of values.
* @property {module:qm~StreamAggrMin} min - The minimal type. Saves the minimal value in the buffer.
* @property {module:qm~StreamAggrMax} max - The maximal type. Saves the maximal value in the buffer.
* @property {module:qm~StreamAggrSparseVecSum} sparse-vec-sum - The sparse-vector-sum type.
* @property {module:qm~StreamAggrMovingAverage} ma - The moving average type. Calculates the average within the window.
* @property {module:qm~StreamAggrEMA} ema - The exponental moving average type. Calculates the exponental average of the values.
* @property {module:qm~StreamAggrEMASpVec} ema-sp-vec - The exponental moving average for sparse vectors type.
* @property {module:qm~StreamAggrMovingVariance} var - The moving variance type. Calculates the variance of values within the window.
* @property {module:qm~StreamAggrMovingCovariance} cov - The moving covariance type. Calculates the covariance of values within the window.
* @property {module:qm~StreamAggrMovingCorrelation} cor - The moving correlation type. Calculates the correlation of values within the window.
* @property {module:qm~StreamAggrResampler} res - The resampler type. Resamples the records so that they come in in the same time interval.
* @property {module:qm~StreamAggrAggrResampler} aggr-res - The aggregating (avg/sum) resampler type. Resamplers the records so that it takes the
* records in the time window and returns one sample.
* @property {module:qm~StreamAggrMerger} mer - The merger type. Merges the records from two stream series.
* @property {module:qm~StreamAggrHistogram} hist - The online histogram type.
* @property {module:qm~StreamAggrSlottedHistogram} slotted-hist - The online slotted-histogram type.
* @property {module:qm~StreamAggrVecDiff} vec-diff - The difference of two vectors (e.g. online histograms) type.
* @property {module:qm~StreamAggrSimpleLinearRegression} lin-reg - The linear regressor type.
* @property {module:qm~StreamAggrAnomalyDetectorNN} detector-nn - The anomaly detector type. Detects anomalies using the k nearest neighbour algorithm.
* @property {module:qm~StreamAggrThreshold} treshold - The threshold indicator type.
* @property {module:qm~StreamAggrTDigest} tdigest - The quantile estimator type. It estimates the quantiles of the given data using {@link module:analytics.TDigest TDigest}.
* @property {module:qm~StreamAggrRecordSwitch} record-switch-aggr - The record switch type.
* @property {module:qm~StreamAggrPageHinkley} pagehinkley - The Page-Hinkley test for concept drift detection type.
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggrTimeSeriesWindow
* This stream aggregator represents the time series window buffer. It stores the values inside a moving window.
* It implements all the stream aggregate methods <b>except</b> {@link module:qm.StreamAggr#getFloat} and {@link module:qm.StreamAggr#getTimestamp}.
* @property {string} name - The given name of the stream aggregator.
* @property {string} type - The type of the stream aggregator. <b>Important:</b> It must be equal to `'timeSeriesWinBuf'`.
* @property {string} store - The name of the store from which to takes the data.
* @property {string} timestamp - The field of the store, where it takes the timestamp.
* @property {string} value - The field of the store, where it takes the values.
* @property {number} winsize - The size of the window, in milliseconds.
* @property {number} delay - Delay in milliseconds.
* @example
* // import the qm module
* var qm = require('qminer');
* // create a base with a simple store
* var base = new qm.Base({
*    mode: "createClean",
*    schema: [
*    {
*        name: "Heat",
*        fields: [
*            { name: "Celsius", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
*
* // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'Celsius' field
* // and the timestamp from the 'Time' field. The size of the window is 2 seconds (2000ms).
* var aggr = {
*    name: 'TimeSeriesAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celsius',
*    winsize: 2000
* };
* base.store("Heat").addStreamAggr(aggr);
* base.close();
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggrTimeSeriesWindowVector
* This stream aggregator represents the values read from a time series window buffer.
* It implements {@link module:qm.StreamAggr#getFloatVector}, {@link module:qm.StreamAggr#getFloatAt} and {@link module:qm.StreamAggr#getFloatLength}.
* @property {string} name - The given name of the stream aggregator.
* @property {string} type - The type of the stream aggregator. <b>Important:</b> It must be equal to `'timeSeriesWinBuf'`.
* @property {string} store - The name of the store from which to takes the data.
* @property {string} inAggr - The name of the window buffer aggregate that represents the input.
* @example
* // import the qm module
* var qm = require('qminer');
* // create a base with a simple store
* var base = new qm.Base({
*    mode: "createClean",
*    schema: [{
*        name: "Heat",
*        fields: [
*            { name: "Celsius", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
*
* var store = base.store("Heat");
* var tick = store.addStreamAggr({
*     type: 'timeSeriesTick',
*     timestamp: 'Time',
*     value: 'Celsius'
* });
*
* var winbufvec = store.addStreamAggr({
*     type: 'timeSeriesWinBufVector',
*     inAggr: tick.name,
*     winsize: 2000
* });
*
* store.push({ Time: '2015-06-10T14:13:32.0', Celsius: 1 });
* winbufvec.getFloatVector().print(); // prints 1
* store.push({ Time: '2015-06-10T14:33:30.0', Celsius: 2 });
* winbufvec.getFloatVector().print(); // prints 2
* store.push({ Time: '2015-06-10T14:33:31.0', Celsius: 3 });
* winbufvec.getFloatVector().print(); // prints 2,3
* store.push({ Time: '2015-06-10T14:33:32.0', Celsius: 4 });
* winbufvec.getFloatVector().print(); // prints 2,3,4
*
* base.close();
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggrRecordBuffer
* This stream aggregator represents record buffer. It stores the values inside a moving window.
* It implements all the stream aggregate methods <b>except</b> {@link module:qm.StreamAggr#getFloat} and {@link module:qm.StreamAggr#getTimestamp}.
* @property {string} name - The given name of the stream aggregator.
* @property {string} type - The type of the stream aggregator. <b>Important:</b> It must be equal to `'recordBuffer'`.
* @property {number} size - The size of the window.
* @example
* // import the qm module
* var qm = require('qminer');
* // create a base with a simple store
* var base = new qm.Base({
*    mode: "createClean",
*    schema: [
*    {
*        name: "Heat",
*        fields: [
*            { name: "Celsius", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
*
* // create a new time series stream aggregator for the 'Heat' store. The size of the window is 3 records.
* var aggr = {
*    name: 'Delay',
*    type: 'recordBuffer',
*    size: 3
* };
* base.store("Heat").addStreamAggr(aggr);
* base.close();
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggrSum
* This stream aggregator represents the sum moving window buffer. It sums all the values, that are in the connected stream aggregator.
* It implements the following methods:
* <br>1. {@link module:qm.StreamAggr#getFloat} returns the sum of the values of the records in its buffer window.
* <br>2. {@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in its buffer window.
* @property {string} name - The given name of the stream aggregator.
* @property {string} type - The type of the stream aggregator. <b>Important:</b> It must be equal to `'winBufSum'`.
* @property {string} store - The name of the store from which it takes the data.
* @property {string} inAggr - The name of the stream aggregator to which it connects and gets data.
* @example
* // import the qm module
* var qm = require('qminer');
* // create a base with a simple store
* var base = new qm.Base({
*    mode: "createClean",
*    schema: [
*    {
*        name: "Income",
*        fields: [
*            { name: "Amount", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
*
* // create a new time series stream aggregator for the 'Income' store, that takes the values from the 'Amount' field
* // and the timestamp from the 'Time' field. The size of the window should 1 week.
* var timeser = {
*    name: 'TimeSeriesAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Income',
*    timestamp: 'Time',
*    value: 'Amount',
*    winsize: 604800000 // 7 days in miliseconds
* };
* var timeSeries = base.store("Income").addStreamAggr(timeser);
*
* // add a sum aggregator, that is connected with the 'TimeSeriesAggr' aggregator
* var sum = {
*    name: 'SumAggr',
*    type: 'winBufSum',
*    store: 'Heat',
*    inAggr: 'TimeSeriesAggr'
* };
* var sumAggr = base.store("Income").addStreamAggr(sum);
* base.close();
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggrMin
* This stream aggregator represents the minimum moving window buffer. It monitors the minimal value in the connected stream aggregator.
* It implements the following methods:
* <br>1. {@link module:qm.StreamAggr#getFloat} returns the minimal value of the records in its buffer window.
* <br>2. {@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in its buffer window.
* @property {string} name - The given name of the stream aggregator.
* @property {string} type - The type of the stream aggregator. <b>Important:</b> It must be equal to `'winBufMin'`.
* @property {string} store - The name of the store from which it takes the data.
* @property {string} inAggr - The name of the stream aggregator to which it connects and gets data.
* @example
* // import the qm module
* var qm = require('qminer');
* // create a base with a simple store
* var base = new qm.Base({
*    mode: "createClean",
*    schema: [
*    {
*        name: "Heat",
*        fields: [
*            { name: "Celsius", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
*
* // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'Celsius' field
* // and the timestamp from the 'Time' field. The size of the window is 1 day.
* var timeser = {
*    name: 'TimeSeriesAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celsius',
*    winsize: 86400000 // 1 day in miliseconds
* };
* var timeSeries = base.store("Heat").addStreamAggr(timeser);
*
* // add a min aggregator, that is connected with the 'TimeSeriesAggr' aggregator
* var min = {
*    name: 'MinAggr',
*    type: 'winBufMin',
*    store: 'Heat',
*    inAggr: 'TimeSeriesAggr'
* };
* var minimal = base.store("Heat").addStreamAggr(min);
* base.close();
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggrMax
* This stream aggregator represents the maximum moving window buffer. It monitors the maximal value in the connected stream aggregator.
* It implements the following methods:
* <br>1. {@link module:qm.StreamAggr#getFloat} returns the maximal value of the records in its buffer window.
* <br>2. {@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in its buffer window.
* @property {string} name - The given name of the stream aggregator.
* @property {string} type - The type for the stream aggregator. <b>Important:</b> It must be equal to `'winBufMax'`.
* @property {string} store - The name of the store from which it takes the data.
* @property {string} inAggr - The name of the stream aggregator to which it connects and gets data.
* @example
* // import the qm module
* var qm = require('qminer');
* // create a base with a simple store
* var base = new qm.Base({
*    mode: "createClean",
*    schema: [
*    {
*        name: "Heat",
*        fields: [
*            { name: "Celsius", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
*
* // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'Celsius' field
* // and the timestamp from the 'Time' field. The size of the window is 1 day.
* var timeser = {
*    name: 'TimeSeriesAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celsius',
*    winsize: 86400000 // one day in miliseconds
* };
* var timeSeries = base.store("Heat").addStreamAggr(timeser);
*
* // add a max aggregator, that is connected with the 'TimeSeriesAggr' aggregator
* var max = {
*    name: 'MaxAggr',
*    type: 'winBufMax',
*    store: 'Heat',
*    inAggr: 'TimeSeriesAggr'
* };
* var maximal = base.store("Heat").addStreamAggr(max);
* base.close();
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggrSparseVecSum
* This stream aggregator represents the sparse-vector-sum moving window buffer. It sums all the sparse-vector values, that are in the connected stream aggregator.
* It implements the following methods:
* <br>1. {@link module:qm.StreamAggr#getValueVector} returns the sum of the values of the records in its buffer window.
* <br>2. {@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in its buffer window.
* @property {string} name - The given name of the stream aggregator.
* @property {string} type - The type of the stream aggregator. <b>Important:</b> It must be equal to `'winBufSpVecSum'`.
* @property {string} store - The name of the store from which it takes the data.
* @property {string} inAggr - The name of the stream aggregator to which it connects and gets data.
* @example
* var qm = require('qminer');
* var base = new qm.Base({
*   mode: 'createClean',
*   schema: [{
*       name: 'Docs',
*       fields: [
*           { name: 'Time', type: 'datetime' },
*           { name: 'Text', type: 'string' }
*       ]
*   }]
* });
* var store = base.store('Docs');
*
* var aggr = {
*   name: 'featureSpaceWindow',
*   type: 'timeSeriesWinBufFeatureSpace',
*   store: 'Docs',
*   timestamp: 'Time',
*   featureSpace: {
*       type: "categorical",
*       source: "Docs",
*       field: "Text"
*   },
*   winsize: 1000
* };
* var sa = store.addStreamAggr(aggr);
*
* var aggr2 = {
*   name: 'sparseVectorSum',
*   type: 'winBufSpVecSum',
*   store: 'Docs',
*   inAggr: 'featureSpaceWindow'
* };
* var sa2 = store.addStreamAggr(aggr2);
*
* store.push({ Time: '2015-06-10T14:13:32.0', Text: 'a' }); // 0
* store.push({ Time: '2015-06-10T14:13:33.0', Text: 'b' }); // 1
* store.push({ Time: '2015-06-10T14:14:34.0', Text: 'c' }); // 2
* store.push({ Time: '2015-06-10T14:15:35.0', Text: 'd' }); // 3
* store.push({ Time: '2015-06-10T14:15:36.0', Text: 'e' }); // 4
* store.push({ Time: '2015-06-10T14:15:37.0', Text: 'f' }); // 5
*
* var valVec2 = sa2.getValueVector(); // [0, 0, 0, 0, 1, 1] - only vectors 4 and 5 remain in window
*
* base.close();
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggrTimeSeriesTick
* This stream aggregator represents the time series tick window buffer. It exposes the data to other stream aggregators
* (similar to {@link module:qm~StreamAggrTimeSeriesWindow}). It implements the following methods:
* <br>1. {@link module:qm.StreamAggr#getFloat} returns the last value added in its buffer window.
* <br>2. {@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in its buffer window.
* @property {string} name - The given name for the stream aggregator.
* @property {string} type - The type of the stream aggregator. <b>Important:</b> It must be equal to `'timeSeriesTick'`.
* @property {string} store - The name of the store from which it takes the data.
* @property {string} value - The name of the store field, from which it takes the values.
* @property {string} timestamp - The name of the store field, from which it takes the timestamp.
* @example
* // import the qm module
* var qm = require('qminer');
* // create a base with a simple store
* var base = new qm.Base({
*    mode: "createClean",
*    schema: [
*    {
*        name: "Students",
*        fields: [
*            { name: "Id", type: "float" },
*            { name: "TimeOfGraduation", type: "datetime" }
*        ]
*    }]
* });
*
* // create a new time series tick stream aggregator for the 'Students' store, that takes the values from the 'Id' field
* // and the timestamp from the 'TimeOfGraduation' field.
* var tick = {
*    name: 'TimeSeriesTickAggr',
*    type: 'timeSeriesTick',
*    store: 'Students',
*    timestamp: 'TimeOfGraduation',
*    value: 'Id',
* };
* var timeSeriesTick = base.store("Students").addStreamAggr(tick);
* base.close();
*/

/**
* @typedef {module:qmStreamAggr} StreamAggrMovingAverage
* This stream aggregator represents the moving average window buffer. It calculates the moving average value of the connected stream aggregator values.
* It implements the following methods:
* <br>1. {@link module:qm.StreamAggr#getFloat} returns the average of the values in its buffer window.
* <br>2. {@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in its buffer window.
* @property {string} name - The given name of the stream aggregator.
* @property {string} type - The type of the stream aggregator. <b>Important:</b> It must be equal to `'ma'`.
* @property {string} store - The name of the store from which it takes the data.
* @property {string} inAggr - The name of the stream aggregator to which it connects and gets data.
* @example
* // import the qm module
* var qm = require('qminer');
* // create a base with a simple store
* var base = new qm.Base({
*    mode: "createClean",
*    schema: [
*    {
*        name: "Heat",
*        fields: [
*            { name: "Celsius", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
*
* // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'Celsius' field
* // and the timestamp from the 'Time' field. The size of the window should be 1 day.
* var timeser = {
*    name: 'TimeSeriesAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celsius',
*    winsize: 86400000
* };
* var timeSeries = base.store("Heat").addStreamAggr(timeser);
*
* // add a moving average aggregator, that is connected with the 'TimeSeriesAggr' aggregator
* var ma = {
*    name: 'movingAverageAggr',
*    type: 'ma',
*    store: 'Heat',
*    inAggr: 'TimeSeriesAggr'
* };
* var movingAverage = base.store("Heat").addStreamAggr(ma);
* base.close();
*/

/**
* @typedef {module:qmStreamAggr} StreamAggrEMA
* This stream aggregator represents the exponential moving average window buffer. It calculates the weighted moving average
* of the values in the connected stream aggregator, where the weights are exponentially decreasing.  It implements the following methods:
* <br>1. {@link module:qm.StreamAggr#getFloat} returns the exponentional average of the values in its buffer window.
* <br>2. {@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in its buffer window.
* @property {string} name - The given name for the stream aggregator.
* @property {string} type - The type of the stream aggregator. <b>Important:</b> It must be equal to `'ema'`.
* @property {string} store - The name of the store from which it takes the data.
* @property {string} inAggr - The name of the stream aggregator to which it connects and gets data.
* It <b>cannot</b> be connect to the {@link module:qm~StreamAggrTimeSeriesWindow}.
* @property {string} emaType - The type of interpolation. Possible options are:
* <br>1. `'previous'` - Interpolates with the previous value.
* <br>2. `'next'` - Interpolates with the next value.
* <br>3. `'linear'` - Makes a linear interpolation.
* @property {number} interval - The time interval defining the decay. It must be greater than `initWindow`.
* @property {number} initWindow - The time window of required values for initialization.
* @example
* // import the qm module
* var qm = require('qminer');
* // create a base with a simple store
* var base = new qm.Base({
*    mode: "createClean",
*    schema: [
*    {
*        name: "Heat",
*        fields: [
*            { name: "Celsius", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
*
* // create a new time series tick stream aggregator for the 'Heat' store, that takes the values from the 'Celsius' field
* // and the timestamp from the 'Time' field. The size of the window should be 1 hour.
* var timeser = {
*    name: 'TimeSeriesAggr',
*    type: 'timeSeriesTick',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celsius'
* };
* var timeSeries = base.store("Heat").addStreamAggr(timeser);
*
* // add an exponentional moving average aggregator, that is connected with the 'TimeSeriesAggr' aggregator.
* // It should interpolate with the previous value, the decay should be 3 seconds and the initWindow should be 2 seconds.
* var ema = {
*    name: 'emaAggr',
*    type: 'ema',
*    store: 'Heat',
*    inAggr: 'TimeSeriesAggr',
*    emaType: 'previous',
*    interval: 3000,
*    initWindow: 2000
* };
* var expoMovingAverage = base.store("Heat").addStreamAggr(ema);
* base.close();
*/

/**
* @typedef {module:qmStreamAggr} StreamAggrThreshold
* This stream aggregator represents a threshold indicator. It outputs 1 if the current value in the data streams is
* above the threshold and 0 otherwise. It implements the following methods:
* <br>1. {@link module:qm.StreamAggr#getFloat} returns the exponentional average of the values in its buffer window.
* <br>2. {@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in its buffer window.
* @property {string} name - The given name for the stream aggregator.
* @property {string} type - The type of the stream aggregator. <b>Important:</b> It must be equal to `'ema'`.
* @property {string} store - The name of the store from which it takes the data.
* @property {string} inAggr - The name of the stream aggregator to which it connects and gets data.
* It <b>cannot</b> be connect to the {@link module:qm~StreamAggrTimeSeriesWindow}.
* @property {string} threshold - The threshold mentioned above.
* @example
* // import the qm module
* var qm = require('qminer');
* // create a base with a simple store
* var base = new qm.Base({
*    mode: "createClean",
*    schema: [
*    {
*        name: "Heat",
*        fields: [
*            { name: "Celsius", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
*
* // create a new time series tick stream aggregator for the 'Heat' store, that takes the values from the 'Celsius' field
* // and the timestamp from the 'Time' field. The size of the window should be 1 hour.
* var timeser = {
*    name: 'TimeSeriesTickAggr',
*    type: 'timeSeriesTick',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celsius'
* };
* var timeSeries = base.store("Heat").addStreamAggr(timeser);
*
* // a threshold aggregator, that is connected wit hthe 'TimeSeriesAggr' aggregator.
* // It should output 1 when the temperature is over 3 degrees Celsius and 0 otherwise
* var thresholdAggr = {
*    name: 'thresholdAggr1',
*    type: 'threshold',
*    store: 'Heat',
*    inAggr: 'TimeSeriesTickAggr',
*    threshold: 3
* };
* var expoMovingAverage = base.store("Heat").addStreamAggr(thresholdAggr);
* base.close();
*/

/**
* @typedef {module:qmStreamAggr} StreamAggrEMASpVec
* This stream aggregator represents the exponential moving average window buffer for sparse vectors. It calculates the weighted moving average
* of the values in the connected stream aggregator, where the weights are exponentially decreasing. It implements the following methods:
* <br>1. {@link module:qm.StreamAggr#getValueVector} returns the exponentional average of the sparse vector values in its buffer window.
* <br>2. {@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in its buffer window.
* @property {string} name - The given name for the stream aggregator.
* @property {string} type - The type of the stream aggregator. <b>Important:</b> It must be equal to `'ema'`.
* @property {string} store - The name of the store from which it takes the data.
* @property {string} inAggr - The name of the stream aggregator to which it connects and gets data.
* It <b>cannot</b> be connect to the {@link module:qm~StreamAggrTimeSeriesWindow}.
* @property {string} emaType - The type of interpolation. Possible options:
* <br>1. `'previous'` - Interpolates with the previous value.
* <br>2. `'next'` - Interpolates with the next value.
* <br>3. `'linear'` - Makes a linear interpolation.
* @property {number} interval - The time interval defining the decay. It must be greater than `initWindow`.
* @property {number} [initWindow=0] - The time window of required values for initialization.
* @property {number} [cuttof=0.001] - Minimal value for any dimension. If value of certain dimension falls bellow this value, the dimension is pruned from average.
* @example
* // import the qm module
* var qm = require('qminer');
* // create a base with a simple store
* var base = new qm.Base({
*    mode: "createClean",
*    schema: [
*    {
*        name: "Data",
*        fields: [
*            { name: "Text", type: "string" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
* var store = base.store("Data");
* // create a new time series that emits data as sparse vector, based on text
* var aggr = {
*    name: 'featureSpaceWindow',
*    type: 'timeSeriesWinBufFeatureSpace',
*    store: store.name,
*    timestamp: 'Time',
*    featureSpace: {
*       type: "categorical",
*       source: store.name,
*       field: "Text"
*    },
*    winsize: 1 // keep only most recent value in window
* };
* // attach sum
* var sa = store.addStreamAggr(aggr);
* var aggr2 = {
*    name: 'sparseVectorSum',
*    type: 'winBufSpVecSum',
*    store: store.name,
*    inAggr: aggr.name // this means that sum is equal to the most recent data
* };
* // ok, now attach EMA
* var sa2 = store.addStreamAggr(aggr2);
* var ema_def = {
*    name: 'sparseVectorEma',
*    type: 'emaSpVec',
*    store: store.name,
*    inAggr: aggr2.name,
*    emaType: "next",
*    interval: 2000,
*    initWindow: 0
* };
* var ema = store.addStreamAggr(ema_def);
* // add some data
* store.push({ Time: 1000, Text: 'a' });
* store.push({ Time: 2000, Text: 'b' });
* store.push({ Time: 3000, Text: 'c' });
* // display EMA data
* ema.getValueVector().print();
* base.close();
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggrMovingVariance
* This stream aggregator represents the moving variance window buffer. It calculates the moving variance of the stream aggregator, that it's connected to.
* It implements the following methods:
* <br>1. {@link module:qm.StreamAggr#getFloat} returns the variance of the values in its buffer window.
* <br>2. {@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in its buffer window.
* @property {string} name - The given name for the stream aggregator.
* @property {string} type - The type of the stream aggregator. <b>Important:</b> It must be equal to `'variance'`.
* @property {string} store - The name of the store from which it takes the data.
* @property {string} inAggr - The name of the stream aggregator to which it connects and gets data.
* @example
* // import the qm module
* var qm = require('qminer');
* // create a base with a simple store
* var base = new qm.Base({
*    mode: "createClean",
*    schema: [
*    {
*        name: "Heat",
*        fields: [
*            { name: "Celsius", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
*
* // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'Celsius' field
* // and the timestamp from the 'Time' field. The size of the window is 1 day
* var timeser = {
*    name: 'TimeSeriesAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celsius',
*    winsize: 86400000
* };
* var timeSeries = base.store("Heat").addStreamAggr(timeser);
*
* // add a variance aggregator, that is connected with the 'TimeSeriesAggr' aggregator
* var variance = {
*    name: 'varAggr',
*    type: 'variance',
*    store: 'Heat',
*    inAggr: 'TimeSeriesAggr'
* };
* var varianceAggr = base.store("Heat").addStreamAggr(variance);
* base.close();
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggrMovingCovariance
* This stream aggregator represents the moving covariance window buffer. It calculates the moving covariance of the two stream aggregators, that it's connected to.
* It implements the following methods:
* <br>1. {@link module:qm.StreamAggr#getFloat} returns the covariance of the values in its buffer window.
* <br>2. {@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in its buffer window.
* @property {string} name - The given name for the stream aggregator.
* @property {string} type - The type of the stream aggregator. <b>Important:</b> It must be equal to `'covariance'`.
* @property {string} store - The name of the store from which it takes the data.
* @property {string} inAggrX - The name of the first stream aggregator to which it connects and gets data.
* @property {string} inAggrY - The name of the recond stream aggregator to which it connects and gets data.
* @example
* // import the qm module
* var qm = require('qminer');
* // create a base with a simple store
* var base = new qm.Base({
*    mode: "createClean",
*    schema: [
*    {
*        name: "Heat",
*        fields: [
*            { name: "Celsius", type: "float" },
*            { name: "WaterConsumption", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
*
* // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'Celsius' field
* // and the timestamp from the 'Time' field. The size of the window is 1 day.
* var Celsius = {
*    name: 'CelsiusAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celsius',
*    winsize: 86400000
* }; base.store("Heat").addStreamAggr(Celsius);
*
* // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'WaterConsumption' field
* // and the timestamp from the 'Time' field. The size of the window is 1 day.
* var water = {
*    name: 'WaterAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'WaterConsumption',
*    winsize: 86400000
* }; base.store("Heat").addStreamAggr(water);
*
* // add a covariance aggregator, that is connected with the 'CelsiusAggr' and 'WaterAggr' stream aggregators
* var covariance = {
*    name: 'covAggr',
*    type: 'covariance',
*    store: 'Heat',
*    inAggrX: 'CelsiusAggr',
*    inAggrY: 'WaterAggr'
* };
* var covarianceAggr = base.store("Heat").addStreamAggr(covariance);
* base.close();
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggrMovingCorrelation
* This stream aggregator represents the moving covariance window buffer. It calculates the moving correlation of the three stream aggregators,
* that it's connected to. It implements the following methods:
* <br>1. {@link module:qm.StreamAggr#getFloat} returns the correlation of the values in it's buffer window.
* <br>2. {@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in it's buffer window.
* @property {string} name - The given name for the stream aggregator.
* @property {string} type - The type of the stream aggregator. <b>Important:</b> It must be equal to `'correlation'`.
* @property {string} store - The name of the store from which it takes the data.
* @property {string} inAggrCov - The name of the covariance stream aggregator.
* @property {string} inAggrVarX - The name of the first variance stream aggregator.
* @property {string} inAggrVarY - The name of the second variance stream aggregator.
* @example
* // import the qm module
* var qm = require('qminer');
* // create a base with a simple store
* var base = new qm.Base({
*    mode: "createClean",
*    schema: [
*    {
*        name: "Heat",
*        fields: [
*            { name: "Celsius", type: "float" },
*            { name: "WaterConsumption", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
*
* // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'Celsius' field
* // and the timestamp from the 'Time' field. The size of the window is 1 day
* var Celsius = {
*    name: 'CelsiusAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celsius',
*    winsize: 86400000
* }; base.store("Heat").addStreamAggr(Celsius);
*
* // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'WaterConsumption' field
* // and the timestamp from the 'Time' field. The size of the window is 1 day
* var water = {
*    name: 'WaterAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'WaterConsumption',
*    winsize: 86400000
* }; base.store("Heat").addStreamAggr(water);
*
* // add a covariance aggregator, that is connected with the 'CelsiusAggr' and 'WaterAggr' aggregators
* var covariance = {
*    name: 'covarianceAggr',
*    type: 'covariance',
*    store: 'Heat',
*    inAggrX: 'CelsiusAggr',
*    inAggrY: 'WaterAggr'
* }; base.store("Heat").addStreamAggr(covariance);
*
* // add the two variance aggregators, that take from the 'Celsius' and 'WaterConsumption' fields, respectively
* var celVar = {
*    name: 'CelsiusVarAggr',
*    type: 'variance',
*    store: 'Heat',
*    inAggr: 'CelsiusAggr'
* }; base.store("Heat").addStreamAggr(celVar);
*
* var waterVar = {
*    name: 'waterVarAggr',
*    type: 'variance',
*    store: 'Heat',
*    inAggr: 'WaterAggr'
* }; base.store("Heat").addStreamAggr(waterVar);
*
* // add a correlation aggregator, that is connected to 'CovarianceAggr', 'CelsiusVarAggr' and 'WaterValAggr' aggregators
* var corr = {
*    name: 'corrAggr',
*    type: 'correlation',
*    store: 'Heat',
*    inAggrCov: 'covarianceAggr',
*    inAggrVarX: 'CelsiusVarAggr',
*    inAggrVarY: 'waterVarAggr'
* };
* var correlation = base.store("Heat").addStreamAggr(corr);
* base.close();
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggrResampler
* This stream aggregator represents the resampler window buffer. It creates new values that are interpolated by using the values from an existing store.
* No methods are implemented for this aggregator.
* @property {string} name - The given name for the stream aggregator.
* @property {string} type - The type of the stream aggregator. <b>Important:</b> It must be equal to `'resampler'`.
* @property {string} store - The name of the store from which it takes the data.
* @property {string} outStore - The store in which the samples are stored.
* @property {string} timestamp - The store field from which it takes the timestamps.
* @property {Array.<Object>} fields - The array off `field` objects from which it takes the values. The `field` object contain the properties:
* <br>`field.name` - The store field from which it takes the values. Type `string`.
* <br>`field.interpolator` - The type of the interpolation. The options are `'previous'`, `'next'` and `'linear'`. Type `string`.
* @property {boolean} createStore - If true, `outStore` must be created.
* @property {number} interval - The interval size. The frequency on which it makes the interpolated values.
* @example
* // import the qm module
* var qm = require('qminer');
* // create a base with a simple store
* var base = new qm.Base({
*    mode: "createClean",
*    schema: [
*    {
*        name: "Heat",
*        fields: [
*            { name: "Celsius", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    },
*    {
*        name: "interpolatedValues",
*        fields: [
*            { name: "Value", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
* // create a new resampler stream aggregator for the 'Heat' store, that takes the values from the 'Celsius' field
* // and the timestamp from the 'Time' field. The interpolated values are stored in the 'interpolatedValues' store.
* // The interpolation should be linear and the interval should be 2 seconds
* var res = {
*    name: 'resamplerAggr',
*    type: 'resampler',
*    store: 'Heat',
*    outStore: 'interpolatedValues',
*    timestamp: 'Time',
*    fields: [{
*        name: 'Celsius',
*        interpolator: 'linear'
*    }],
*    createStore: false,
*    interval: 2000
* };
* var resampler = base.store("Heat").addStreamAggr(res);
* base.close();
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggrAggrResampler
* This stream aggregate resamples an input time series to a new time seris
* of equally spaced measurements. Each new measurement corresponds to an
* aggregate (sum,avg,min,max) computed over an interval. The aggregate
* exposes the following methods.
* <br>1. {@link module:qm.StreamAggr#getFloat} returns the last resampled value.
* <br>2. {@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the last resampled value.
* <br>3. {@link module:qm.StreamAggr#onStep} reads from an input aggregate and tries to resample.
* <br>4. {@link module:qm.StreamAggr#onTime} updates the current time (no data has arrived, but time has passed) and tries to resample.
* <br>5. {@link module:qm.StreamAggr#getParams} returns a parameter object.
* <br>6. {@link module:qm.StreamAggr#setParams} used primarily for setting the out-aggregate.
* <br>The stream aggregate exposes its results through `getFloat` and `getTimestamp` methods (itself represents timeseries).
* The resampler has an input time-series aggregate (supports `getFloat` and `getTimestamp`), from where it reads time series values.
* The reading and resampling occourrs wehen resamplers `onStep()` or `onTime()` methods are called.
* When resampling succeeds (all the data needed for the computation becomes available), the resampler
* will trigger the `onStep()` method of an output stream aggregate that will read the resamplers state through `getFloat` and `getTime`.
* @property {string} type - The type of the stream aggregator. <b>Important:</b> It must be equal to `'aggrResampler'`.
* @property {number} interval - Interval size in milliseconds
* @property {string} aggType - Must be one of the values: `"sum"`, `"avg"`, `"min"` or `"max"` - represents the function executed on the data values in the interval.
* @property {(string | module:qm.StreamAggr)} inAggr - The name of the input stream aggregate which must implement `getFloat()` and `getTimestamp()` methods.
* @property {(string | number)} [start] - Start time (linux timestamp or a web log date string like `1970-01-01T00:00:00.000`)
* @property {string} [roundStart] - Must be one of the values: `'h'`, `'m'` or `'s'` - represents rounding of the start time when it must be determined by the first observed record. `'h'` will clip minutes, seconds and milliseconds, `'m'` will clip seconds and milliseconds and `'s'` will clip only milliseconds.
* @property {number} [defaultValue=0] - default value for empty intervals (no data available).
* @property {boolean} [skipEmpty=false] - If true, the resampler will not call the `onStep` method of the out-aggregate when the interval is empty (for example, average of an empty set is not defined).
* @property {string} [name] - The given name for the stream aggregator.
* @property {(string | module:qm.StreamAggr)} [outAggr] - The name of the output stream aggregate. Only useful when the `outAggr` is a javascript stream aggregate, otherwise the output must be set by calling `setParam({outAggr: outAggregateName})`.
* @example
* var qm = require('qminer');
* // create a base with a simple timeseries store
* var base = new qm.Base({
*     mode: 'createClean',
*     schema: [{
*         name: 'default',
*         fields: [
*             { name: 'timestamp', type: 'datetime' },
*             { name: 'value', type: 'float' }
*         ]
*     }]
* });
* var store = base.store('default');
* // the tick aggregate reads from the store (provides time series input to other aggregates)
* var raw = store.addStreamAggr({
*     type: 'timeSeriesTick',
*     timestamp: 'timestamp',
*     value: 'value'
* });
*
* // will compute sums over 1 second intervals
* var resampler = store.addStreamAggr({
*     type: 'aggrResample',
*     inAggr: raw.name,
*     start: '1970-01-01T00:00:00.000',
*     defaultValue: 0,
*     aggType: 'sum',
*     interval: 1000
* });
*
* // will print out resampler state on each resample
* var resamplerOutput = new qm.StreamAggr(base, new function () {
*     this.onStep = function () {
*         console.log('Resampler emitted the sum: ' + resampler.getFloat() +
*             ' for the interval [' + new Date(resampler.getTimestamp()).toISOString() +
*             ' - ' + new Date(resampler.getTimestamp() + resampler.getParams().interval).toISOString() + ')');
*     }
* });
*
* // IMPORTANT. After the output exists, connect it to resampler
* resampler.setParams({ outAggr: resamplerOutput.name });
*
* store.push({ timestamp:  1, value: 1 });
* store.push({ timestamp: 10, value: 10 });
* store.push({ timestamp: 500, value: 100 });
* store.push({ timestamp: 2000, value: 1000 }); // triggers two resampling steps (two intervals complete)
* // // three measurements for the first interval
* // Resampler emitted the sum: 111 for the interval [1970-01-01T00:00:00.000Z - 1970-01-01T00:00:01.000Z)
* // // zero measurements for the second interval (does not skip empty)
* // Resampler emitted the sum: 0 for the interval [1970-01-01T00:00:01.000Z - 1970-01-01T00:00:02.000Z)
* store.push({ timestamp: 2001, value: 10000 });
* store.push({ timestamp: 3000, value: 100000 }); // triggers one resampling step (one interval complete)
* // // one measurement for the third interval
* // Resampler emitted the sum: 11000 for the interval [1970-01-01T00:00:02.000Z - 1970-01-01T00:00:03.000Z)
*
* base.close();
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggrMerger
* This stream aggregator represents the merger aggregator. It merges records from two or more stores into a new store
* depending on the timestamp. No methods are implemented for this aggregator.
* <image src="pictures/merger.gif" alt="Merger Animation">
* @property {string} name - The given name for the stream aggregator.
* @property {string} type - The type of the stream aggregator. <b>Important:</b> It must be equal to `'merger'`.
* @property {string} outStore - The name of the store where it saves the merged records.
* @property {boolean} createStore - If the outStore must be created.
* @property {string} timestamp - The store field of outStore, where the timestamp is saved.
* @property {Array.<Object>} fields - An array of `field` objects. The `field` object contain the properties:
* <br>`field.source` - The name of the store, from which it takes the values. Type `string`.
* <br>`field.inField` - The field name of source, from which it takes the values. Type `string`.
* <br>`field.outField` - The field name of outStore, into which it saves the values. Type `string`.
* <br>`field.interpolation` - The type of the interpolation. The options are: `'previous'`, `'next'` and `'linear'`. Type `string`.
* <br>`field.timestamp` - The field name of source, where the timestamp is saved. Type `string`.
* @example
* // import the qm module
* var qm = require('qminer');
* // create a base with a simple store
* var base = new qm.Base({
*    mode: "createClean",
*    schema: [
*    {
*        name: "Cars",
*        fields: [
*            { name: "NumberOfCars", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    },
*    {
*        name: "Temperature",
*        fields: [
*            { name: "Celsius", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    },
*    {
*        name: "Merged",
*        fields: [
*            { name: "NumberOfCars", type: "float" },
*            { name: "Celsius", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
* // create a new merger stream aggregator that mergers the records of the 'Cars' and 'Temperature' stores.
* // The records are interpolated linearly and stored in the 'Merged' store.
* var mer = {
*    name: 'MergerAggr',
*    type: 'merger',
*    outStore: 'Merged',
*    createStore: false,
*    timestamp: 'Time',
*    fields: [
*        { source: 'Cars', inField: 'NumberOfCars', outField: 'NumberOfCars', interpolation: 'linear', timestamp: 'Time' },
*        { source: 'Temperature', inField: 'Celsius', outField: 'Celsius', interpolation: 'linear', timestamp: 'Time' }
*    ]
* };
* var merger = new qm.StreamAggr(base, mer);
* base.close();
*/

/**
* @typedef {module:qmStreamAggr} StreamAggrFeatureSpace
* This stream aggregator creates the feature space and stores the specified features of the last input. It implements the following methods:
* <br>1. {@link module:qm.StreamAggr#getFloatVector} returns the dense feature vectors.
* <br>2. {@link module:qm.StreamAggr#getFeatureSpace} returns the feature space.
* @property {string} name - The given name for the stream aggregator.
* @property {string} type - The type of the stream aggregator. <b>Important:</b> It must be equal to `'featureSpace'`.
* @property {string} store - The name of the store from which it takes the data.
* @property {number} initCount - The number of records needed before it initializes.
* @property {boolean} update - If true, updates the feature space.
* @property {boolean} full - If true, saves the full vector of features.
* @property {boolean} sparse - If true, saves the sparse vector of features.
* @property {module:qm~FeatureExtractor[]} FeatureSpace - Array of feature extractors.
* @example
* // import the qm module
* var qm = require('qminer');
* // create a base with a simple store named Cars with 4 fields
* var base = new qm.Base({
*     mode: 'createClean',
*     schema: [{
*         name: 'Cars',
*         fields: [
*             { name: 'NumberOfCars', type: 'float' },
*             { name: 'Temperature', type: 'float' },
*             { name: 'Precipitation', type: 'float' },
*             { name: 'Time', type: 'datetime' }
*         ]
*     }]
* });
* // create the store
* var store = base.store('Cars');
* // define a feature space aggregator on the Cars store which needs at least 2 records to be initialized. Use three of the
* // four fields of the store to create feature vectors with normalized values.
* var aggr = {
*    name: "ftrSpaceAggr",
*    type: "featureSpace",
*    initCount: 2,
*    update: true, full: false, sparse: true,
*    featureSpace: [
*        { type: "numeric", source: "Cars", field: "NumberOfCars", normalize: "var" },
*        { type: "numeric", source: "Cars", field: "Temperature", normalize: "var" },
*        { type: "numeric", source: "Cars", field: "Precipitation", normalize: "var" }
*    ]
* };
* //create the feature space aggregator
* var ftrSpaceAggr = base.store('Cars').addStreamAggr(aggr);
* base.close();
*/

/**
* @typedef {module:qmStreamAggr} StreamAggrAnomalyDetectorNN
* This stream aggregator represents the anomaly detector using the Nearest Neighbor algorithm. It calculates the
* new incoming point's distance from its nearest neighbor and, depending on the input threshold values, it
* classifies the severity of the alarm. It implements the following methods:
* <br>1. {@link module:qm.StreamAggr#getInteger} returns the severity of the alarm.
* <br>2. {@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the latest alarm.
* <br>3. {@link module:qm.StreamAggr#saveJson} returns the Json with the description and explanation of the alarm.
* @property {string} name - The given name for the stream aggregator.
* @property {string} type - The type of the stream aggregator. <b>Important:</b> It must be equal to `'nnAnomalyDetector'`.
* @property {string} store - The name of the store from which it takes the data.
* @property {string} inAggr - The name of the stream aggregator to which it connects and gets data.
* It <b>cannot</b> be connect to the {@link module:qm~StreamAggrTimeSeriesWindow}.

* @example
* // import the qm module
* var qm = require('qminer');
* // create a base with a simple store named Cars with 4 fields
* var base = new qm.Base({
*     mode: 'createClean',
*     schema: [{
*         name: 'Cars',
*         fields: [
*             { name: 'NumberOfCars', type: 'float' },
*             { name: 'Temperature', type: 'float' },
*             { name: 'Precipitation', type: 'float' },
*             { name: 'Time', type: 'datetime' }
*         ]
*     }]
* });
* // create the store
* var store = base.store('Cars');
* // define a feature space aggregator on the Cars store which needs at least 2 records to be initialized. Use three of the
* // four fields of the store to create feature vectors with normalized values.
* var aggr = {
*    name: "ftrSpaceAggr",
*    type: "featureSpace",
*    initCount: 2,
*    update: true, full: false, sparse: true,
*    featureSpace: [
*        { type: "numeric", source: "Cars", field: "NumberOfCars", normalize: "var" },
*        { type: "numeric", source: "Cars", field: "Temperature", normalize: "var" },
*        { type: "numeric", source: "Cars", field: "Precipitation", normalize: "var" }
*    ]
* };
* //create the feature space aggregator
* var ftrSpaceAggr = base.store('Cars').addStreamAggr(aggr);

* // define a new time series tick stream aggregator for the 'Cars' store, that takes the values from the 'NumberOfCars' field
* // and the timestamp from the 'Time' field.
* var aggr = {
*     name: "tickAggr",
*     type: "timeSeriesTick",
*     store: "Cars",
*     timestamp: "Time",
*     value: "NumberOfCars"
* };
* //create the tick aggregator
* var tickAggr = base.store('Cars').addStreamAggr(aggr);
*
* //define an anomaly detection aggregator using nearest neighbor on the cars store that takes as input timestamped features.
* // The time stamp is provided by the tick aggregator while the feature vector is provided by the feature space aggregator.
* var aggr = {
*     name: 'AnomalyDetectorAggr',
*     type: 'nnAnomalyDetector',
*     inAggrSpV: 'ftrSpaceAggr',
*     inAggrTm: 'tickAggr',
*     rate: [0.7, 0.5, 0.15],
*     windowSize: 2
* };
* //create the anomaly detection aggregator
* var anomaly = base.store('Cars').addStreamAggr(aggr);
* base.close();
*/


/**
* @typedef {module:qm.StreamAggr} StreamAggrHistogram
* This stream aggregator represents an online histogram. It can connect to a buffered aggregate (such as {@link module:qm~StreamAggrTimeSeriesWindow})
* or a time series (such as {@link module:qm~StreamAggregateEMA}).
* The aggregate defines an ordered set of points `p(0), ..., p(n)` that define n bins. Infinites at both ends are allowed.
* A new measurement is tested for inclusion in the left-closed right-opened intervals `[p(i), p(i+1))` and the corresponding
* bin counter is increased for the appropriate bin (or decreased if the point is outgoing from the buffer).
* It implements the following methods:
* <br>1. {@link module:qm.StreamAggr#getFloatLength} returns the number of bins.
* <br>2. {@link module:qm.StreamAggr#getFloatAt} returns the count for a bin index.
* <br>3. {@link module:qm.StreamAggr#getFloatVector} returns the vector of counts, the length is equal to the number of bins.
* @property {string} name - The given name of the stream aggregator.
* @property {string} type - The type for the stream aggregator. <b>Important:</b> It must be equal to `'onlineHistogram'`.
* @property {string} store - The name of the store from which it takes the data.
* @property {string} inAggr - The name of the stream aggregator to which it connects and gets data.
* @property {number} lowerBound - The lowest non-infinite bin point.
* @property {number} upperBound - The highest non-infinite bin point.
* @property {number} [bins=5] - The number of bins bounded by `lowerBound` and `upperBound`.
* @property {boolean} [addNegInf=false] - Include a bin `[-Inf, lowerBound]`.
* @property {boolean} [addPosInf=false] - Include a bin `[upperBound, Inf]`.
* @property {boolean} [autoResize=false] - The histogram will be empty at the beginning and double its size on demand (resize only when incrementing counts). The unbounded bins are guaranteed to stay between lowerBound and upperBound and in all cases the bin size equals (upperBound - lowerBound)/bins.
* @example
* // import the qm module
* var qm = require('qminer');
* // create a base with a simple store
* var base = new qm.Base({
*    mode: "createClean",
*    schema: [
*    {
*        name: "Heat",
*        fields: [
*            { name: "Celsius", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
*
* // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'Celsius' field
* // and the timestamp from the 'Time' field. The size of the window is 1 day.
* var timeser = {
*    name: 'TimeSeriesBuffer',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celsius',
*    winsize: 86400000 // one day in miliseconds
* };
* var timeSeries = base.store("Heat").addStreamAggr(timeser);
*
* // add a histogram aggregator, that is connected with the 'TimeSeriesAggr' aggregator
* var aggrJson = {
*    name: 'Histogram',
*    type: 'onlineHistogram',
*    store: 'Heat',
*    inAggr: 'TimeSeriesBuffer',
*    lowerBound: 0,
*    upperBound: 10,
*    bins: 5,
*    addNegInf: false,
*    addPosInf: false
* };
* var hist = base.store("Heat").addStreamAggr(aggrJson);
* base.close();
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggrSlottedHistogram
* This stream aggregator represents an online slotted histogram. It can connect to a buffered aggregate (such as {@link module:qm~StreamAggrTimeSeriesWindow})
* or a time series (such as {@link module:qm~StreamAggregateEMA}).
* It maps historical values into single period (e.g. into hours of the week).
* The aggregate defines an ordered set of points `p(0), ..., p(n)` that define n bins. Infinites at both ends are NOT allowed.
* A new measurement is tested for inclusion in the left-closed right-opened intervals `[p(i), p(i+1))` and the corresponding
* bin counter is increased for the appropriate bin (or decreased if the point is outgoing from the buffer).
* It implements the following methods:
* <br>1. {@link module:qm.StreamAggr#getFloatLength} returns the number of bins.
* <br>2. {@link module:qm.StreamAggr#getFloatAt} returns the count for a bin index.
* <br>3. {@link module:qm.StreamAggr#getFloatVector} returns the vector of counts, the length is equal to the number of bins.
* @property {string} name - The given name of the stream aggregator.
* @property {string} type - The type for the stream aggregator. <b>Important:</b> It must be equal to `'onlineHistogram'`.
* @property {string} store - The name of the store from which it takes the data.
* @property {string} inAggr - The name of the stream aggregator to which it connects and gets data.
* @property {number} period - Cycle length in miliseconds.
* @property {number} window - Window length that is reported when aggregate is queried.
* @property {number} bins - The number of bins - input data is expected to be withing interval `[0, bins-1]`.
* @property {number} granularity - Storage granularity in miliseconds. History is stored in slots with this length. Number of slots is equal to period/granularity.
* @example
* // import the qm module
* var qm = require('qminer');
* // create a base with a simple store
* var base = new qm.Base({
*    mode: "createClean",
*    schema: [
*    {
*        name: "Heat",
*        fields: [
*            { name: "Celsius", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
*
* // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'Celsius' field
* // and the timestamp from the 'Time' field. The size of the window is 4 weeks
* var timeser = {
*    name: 'TimeSeriesBuffer',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celsius',
*    winsize: 2419200000 // 4 weeks
* };
* var timeSeries = base.store("Heat").addStreamAggr(timeser);
*
* // add a slotted-histogram aggregator, that is connected with the 'TimeSeriesAggr' aggregator
* // it will present accumulated histogram for the last 2 hours (window) of the week (period) for the last 4 weeks (see aggregate above)
* var aggrJson = {
*    name: 'Histogram',
*    type: 'onlineSlottedHistogram',
*    store: 'Heat',
*    inAggr: 'TimeSeriesBuffer',
*    period: 604800000, // 1 week
*    window: 7200000, // 2h
*    bins: 5, // 5 possible clusters
*    granularity: 300000  // 5 min
* };
* var hist = base.store("Heat").addStreamAggr(aggrJson);
* base.close();
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggrVecDiff
* This stream aggregator represents difference between two vectors (e.g. online histograms).
* It implements the following methods:
* <br>1. {@link module:qm.StreamAggr#getFloatLength} returns the number of bins.
* <br>2. {@link module:qm.StreamAggr#getFloatAt} returns the count for a bin index.
* <br>3. {@link module:qm.StreamAggr#getFloatVector} returns the vector of counts, the length is equal to the number of bins.
* @property {string} name - The given name of the stream aggregator.
* @property {string} type - The type for the stream aggregator. <b>Important:</b> It must be equal to `'onlineVecDiff'`.
* @property {string} storeX - The name of the store from which it takes the data for the first vector.
* @property {string} storeY - The name of the store from which it takes the data for the second vector.
* @property {string} inAggrX - The name of the first stream aggregator to which it connects and gets data.
* @property {string} inAggrY - The name of the second stream aggregator to which it connects and gets data.
* @example
* // import the qm module
* var qm = require('qminer');
* // create a base with a simple store
* // the store records results of clustering
* var base = new qm.Base({
* mode: "createClean",
* schema: [
* {
*   name: "Rpm",
*   fields: [
*       { name: "ClusterId", type: "float" },
*       { name: "Time", type: "datetime" }
*   ]
* }]
* });
*
* var store = base.store('Rpm');
*
* // create a new time series stream aggregator for the 'Rpm' store that takes the recorded cluster id
* // and the timestamp from the 'Time' field. The size of the window is 4 weeks.
* var timeser1 = {
*   name: 'TimeSeries1',
*   type: 'timeSeriesWinBuf',
*   store: 'Rpm',
*   timestamp: 'Time',
*   value: 'ClusterId',
*   winsize: 7200000 // 2 hours
* };
* var timeSeries1 = base.store("Rpm").addStreamAggr(timeser1);
*
* // add a histogram aggregator, that is connected with the 'TimeSeries1' aggregator
* var aggrJson1 = {
*   name: 'Histogram1',
*   type: 'onlineHistogram',
*   store: 'Rpm',
*   inAggr: 'TimeSeries1',
*   lowerBound: 0,
*   upperBound: 5,
*   bins: 5,
*   addNegInf: false,
*   addPosInf: false
* };
* var hist1 = base.store("Rpm").addStreamAggr(aggrJson1);
*
* // create a new time series stream aggregator for the 'Rpm' store that takes the recorded cluster id
* // and the timestamp from the 'Time' field.
* var timeser2 = {
*   name: 'TimeSeries2',
*   type: 'timeSeriesWinBuf',
*   store: 'Rpm',
*   timestamp: 'Time',
*   value: 'ClusterId',
*   winsize: 21600000 // 6 hours
* };
* var timeSeries2 = base.store("Rpm").addStreamAggr(timeser2);
*
* // add a histogram aggregator, that is connected with the 'TimeSeries1' aggregator
* var aggrJson2 = {
*   name: 'Histogram2',
*   type: 'onlineHistogram',
*   store: 'Rpm',
*   inAggr: 'TimeSeries2',
*   lowerBound: 0,
*   upperBound: 5,
*   bins: 5,
*   addNegInf: false,
*   addPosInf: false
* };
* var hist2 = base.store("Rpm").addStreamAggr(aggrJson2);
*
* // add diff aggregator that subtracts Histogram1 with 2h window from Histogram2 with 6h window
* var aggrJson3 = {
*   name: 'DiffAggr',
*   type: 'onlineVecDiff',
*   storeX: 'Rpm',
*   storeY: 'Rpm',
*   inAggrX: 'Histogram2',
*   inAggrY: 'Histogram1'
* }
* var diff = store.addStreamAggr(aggrJson3);
* base.close();
*/

/**
* @typedef {Object} StreamAggrSimpleLinearRegressionResult
* Simple linear regression result JSON returned by {@link module:qm~StreamAggrSimpleLinearRegression}.
* @property {number} intercept - Regressor intercept.
* @property {number} slope - Regressor slope.
* @property {Array<number>} [quantiles] - Quantiles for which bands will be computed.
* @property {Array<number>} [bands] - Computed band intercepts.
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggrSimpleLinearRegression
* This stream aggregator computes a simple linear regression given two stream aggregates
* that support the {@link module:qm.StreamAggr#getFloatVector} method and represent variates (input) and covariates (output).
* Optionally the aggregate computes quantile bands: for each quantile `q` a parallel line to the fitted
* line is found, so that `q` fraction of `(x,y)` datapoints fall below the line. For example, under Gaussian noise,
* if `Y = a + k X + N(0,sig)` then the 0.95 quantile band will equal `a + 2 sig`. This means that 95% of `(x,y)` pairs
* lie below the line `Y = a + 2 sig + k X`.
* The results are returned as a JSON by calling {@link module:qm.StreamAggr#saveJson} and are of type {@link module:qm~StreamAggrSimpleLinearRegressionResult}.
*
* @property {string} name - The given name of the stream aggregator.
* @property {string} type - The type for the stream aggregator. <b>Important:</b> It must be equal to `'onlineVecDiff'`.
* @property {string} storeX - The name of the store from which it takes the data for the first vector.
* @property {string} storeY - The name of the store from which it takes the data for the second vector.
* @property {string} inAggrX - The name of the first stream aggregator to which it connects and gets data.
* @property {string} inAggrY - The name of the second stream aggregator to which it connects and gets data.
* @property {Array.<number>} quantiles - An array of numbers between 0 and 1 for which the quantile bands will be computed.
* @example
* // import the qm module
* var qm = require('qminer');
* // create a base with X,Y measurements
* var base = new qm.Base({
*     mode: 'createClean',
*     schema: [{
*         name: 'Function',
*         fields: [
*             { name: 'Time', type: 'datetime' },
*             { name: 'X', type: 'float' },
*             { name: 'Y', type: 'float' }
*         ]
*     }]
* });
*
* var store = base.store('Function');
*
* // 1 second buffer for X values
* var winX = store.addStreamAggr({
*     type: 'timeSeriesWinBuf',
*     timestamp: 'Time',
*     value: 'X',
*     winsize: 1000
* });
* // 1 second buffer for Y values
* var winY = store.addStreamAggr({
*     type: 'timeSeriesWinBuf',
*     timestamp: 'Time',
*     value: 'Y',
*     winsize: 1000
* });
*
* // the will find regression line, as well as two parallel quartile lines
* var linReg = store.addStreamAggr({
*     type: 'simpleLinearRegression',
*     inAggrX: winX.name,
*     inAggrY: winY.name,
*     storeX: "Function",
*     storeY: "Function",
*     quantiles: [0.25, 0.75]
* });
*
* store.push({ Time: '2015-06-10T14:13:32.001', X: 0, Y: -2 });
* store.push({ Time: '2015-06-10T14:13:32.002', X: 0, Y: -1 });
* store.push({ Time: '2015-06-10T14:13:32.003', X: 0, Y: 1 });
* store.push({ Time: '2015-06-10T14:13:32.004', X: 0, Y: 2 });
* store.push({ Time: '2015-06-10T14:13:32.005', X: 1, Y: -1 });
* store.push({ Time: '2015-06-10T14:13:32.006', X: 1, Y: -0 });
* store.push({ Time: '2015-06-10T14:13:32.007', X: 1, Y: 2 });
* store.push({ Time: '2015-06-10T14:13:32.008', X: 1, Y: 3 });
*
* var res = linReg.saveJson();
* res.bands[0]; // -1.5
* res.bands[1]; // 1.5
*
* base.close();
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggrTDigest
* This stream aggregator computes the quantile estimators using the {@link module:analytics.TDigest TDigest} algorithm.
* The quantile values are returned using {@link module:qm.StreamAggr#getFloatVector}.
*
* @property {string} name - The given name of the stream aggregator.
* @property {string} type - The type for the stream aggregator. <b>Important:</b> It must be equal to `'tdigest'`.
* @property {string} store - The name of the store from which it takes the data.
* @property {string} inAggr - The name of the stream aggregator to which it connects and gets data.
* @property {Array.<number>} quantiles - An array of numbers between 0 and 1 for which the quantile bands will be computed.
* @property {Number} minCount - The minimal number of values given before it start to compute the quantiles.
* @example
* // import the qm module
* var qm = require('qminer');
* // create a base with the Time and Value fields
* var base = new qm.Base({
*    mode: "createClean",
*    schema: [
*    {
*        name: "Processor",
*        fields: [
*            { name: "Value", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
* var store = base.store('Processor');
*
* // create a new time series stream aggregator for the 'Processor' store, that takes the value of the processor
* // and the timestamp from the 'Time' field. The size of the window is 1 second.
* var tick = {
*     name: 'TickAggr',
*     type: 'timeSeriesTick',
*     store: 'Processor',
*     timestamp: 'Time',
*     value: 'Value',
*     winsize: 1000 // one day in miliseconds
* };
* var timeSeries = store.addStreamAggr(tick);
*
* // create the TDigest stream aggregator
* var aggr = {
*     name: 'TDigest',
*     type: 'tdigest',
*     store: 'Processor',
*     inAggr: 'TickAggr',
*     quantiles: [0.90, 0.95, 0.99, 0.999],
*     minCount: 5
* };
* // add the stream aggregator to the 'Processor' store
* var td = store.addStreamAggr(aggr);
* store.push({ Time: '2015-12-01T14:20:32.0', Value: 0.9948628368 });
* store.push({ Time: '2015-12-01T14:20:33.0', Value: 0.1077458826 });
* store.push({ Time: '2015-12-01T14:20:34.0', Value: 0.9855685823 });
* store.push({ Time: '2015-12-01T14:20:35.0', Value: 0.7796449082 });
* // with this record the aggregator will initialize becuase it is the 5th record
* store.push({ Time: '2015-12-01T14:20:36.0', Value: 0.0844943286 });
*
* store.push({ Time: '2015-12-01T14:20:37.0', Value: 0.187490856 });
* store.push({ Time: '2015-12-01T14:20:38.0', Value: 0.0779815107 });
* store.push({ Time: '2015-12-01T14:20:39.0', Value: 0.8945312691 });
* store.push({ Time: '2015-12-01T14:20:40.0', Value: 0.5574567409 });
*
* // get the quantile estimations
* var result = td.getFloatVector();
* base.close();
*/

/**
 * @typedef {module:qm.StreamAggr} StreamAggrWindowQuantiles
 * This stream aggregate computes approximate quantiles on a sliding time window using
 * the SW-GK algorithm proposed in:
 * http://dl.acm.org/citation.cfm?id=2954329
 *
 * The quantile values are returned using {@link module:qm.StreamAggr#getFloatVector}.
 *
 * @property {string} name - The given name of the stream aggregator.
 * @property {string} type - Must use type 'windowQuantiles'.
 * @property {string} inAggr - The name of the stream aggregate which defines the time window.
 * @property {Array.<number>} quantiles - An array of p-values for which the algorithm will return quantiles.
 * @property {number} quantileEps - Maximal relative error of the quantile estimation procedure.
 * @property {number} countEps - Maximal relative error of the count procedure.
 *
 * If the number of items in the window is N, then the algorithms error is bound by
 * N*(quantileEps + 2*countEps + O(countEps^2)). However in practice, the error should
 * be less.
 *
 * @example
 *
 * var assert = require('assert');
 * var qm = require('qminer');
 *
 * // variables
 * var batchSize = 1000;
 * var nbatches = 10;
 *
 * var dt = 10;
 * var windowMSec = batchSize*dt;
 *
 * var quantileEps = 0.01;
 * var countEps = 0.0001;
 *
 * var maxRelErr = quantileEps + 2*countEps;
 *
 * var targetQuants = (function () {
 *     var quants = [];
 *     for (var prob = 0; prob <= 1; prob += 0.001) {
 *         quants.push(prob);
 *     }
 *     return quants;
 * })();
 *
 * // create a base with a simple store
 * // the store records results of throwing two independent fair dices
 * var base = new qm.Base({
 *     mode: "createClean",
 *     schema: [
 *     {
 *         name: "hugeDie",
 *         fields: [
 *             { name: "value", type: "float" },
 *             { name: "time", type: "datetime" }
 *         ]
 *     }]
 * });
 * var store = base.store('hugeDie');
 *
 * // create a new time series stream aggregator for the 'Dice' store, that takes the expected values of throwing a dice
 * // and the timestamp from the 'Time' field. The size of the window is 1 day.
 * var windowAggr = store.addStreamAggr({
 *     name: 'TimeSeries1',
 *     type: 'timeSeriesWinBuf',
 *     store: store,
 *     timestamp: 'time',
 *     value: 'value',
 *     winsize: windowMSec
 * });
 *
 * var gk = store.addStreamAggr({
 * type: 'windowQuantiles',
 *     inAggr: windowAggr,
 *     quantileEps: quantileEps,
 *     countEps: countEps,
 *     quantiles: targetQuants
 * })
 *
 * var vals = [];
 * for (var i = 0; i < batchSize; i++) {
 *     vals.push(i);
 * }
 * for (var batchN = 0; batchN < nbatches; batchN++) {
 *     // shuffle the array
 *     for (var i = 0; i < batchSize; i++) {
 *         var swapN = Math.floor(Math.random()*batchSize);
 *         var temp = vals[i];
 *         vals[i] = vals[swapN];
 *         vals[swapN] = temp;
 *     }
 *
 *     for (var i = 0; i < batchSize; i++) {
 *         var time = (batchN*batchSize + i)*dt;
 *         store.push({ time: time, value: vals[i] })
 *     }
 *
 *     var result = gk.getFloatVector();
 *     for (var i = 0; i < targetQuants.length; i++) {
 *         var pval = targetQuants[i];
 *         var quant_hat = result[i];
 *         assert(Math.floor((pval - maxRelErr)*batchSize) <= quant_hat);
 *         assert(Math.ceil((pval + maxRelErr)*batchSize) >= quant_hat);
 *     }
 * }
 */

/**
* @typedef {module:qm.StreamAggr} StreamAggrRecordSwitch
* This stream aggregate enables switching control flow between stream aggregates based
* on string keys. It is based on a hash table from keys (strings read from records) to
* stream aggregates. When OnAdd of a record switch is called, the aggregate looks for
* an appropriate target aggregate and triggers its onAdd. The aggregate
* exposes the following methods.
* <br>1. {@link module:qm.StreamAggr#getInteger} takes a string input and returns returns 1 if the string is a known key and `null` if it's unknown.
* <br>2. {@link module:qm.StreamAggr#onAdd} reads the appropriate field as a key and triggers the onAdd of a target aggregate if the key is known.
* <br>3. {@link module:qm.StreamAggr#getParams} returns a parameter object.
* <br>4. {@link module:qm.StreamAggr#setParams} used primarily for adding (using $add) new targets or seting  (using $set) the internal hashmap with new targets.
*
* @property {string} [name] - The given name of the stream aggregator (autogenerated by default).
* @property {string} type - The type for the stream aggregator. <b>Important:</b> It must be equal to `'recordSwitchAggr'`.
* @property {string} store - The name of the store consistent with the records that it processes.
* @property {string} fieldName - The name of the field whose values are used for switching
* @property {boolean} [throwMissing=false] - If true, the aggregate will throw an exception when the record's key is not recognized (no appropirate target aggregate exists).
* @property {Array.<Object>} [$set] - An array with objects like `{'key': string, 'aggrName': string}`. Each object is a switch key and the name of the target aggregate.
* @example
* // main library
* var qm = require('qminer');
*
* // create a store
* var base = new qm.Base({
*     mode: 'createClean',
*     schema: [{
*         name: 'testStore',
*         fields: [
*             { name: 'switchField', type: 'string' }
*         ]
*     }]
* });
* // select store
* var store = base.store('testStore');
*
* // first JS aggregate
* var outAggr1 = new qm.StreamAggr(base, new function () {
*     this.onAdd = function (rec) {
*         console.log('first');
*     }
* });
*
* // second JS aggregate
* var outAggr2 = new qm.StreamAggr(base, new function () {
*     this.onAdd = function (rec) {
*         console.log('second');
*     }
* });
*
* // switcher aggregate: calls outAggr1 when rec.switchField == 'a' and outAggr2 when rec.switchField == 'b'
* var switcher = store.addStreamAggr({
*     type: 'recordSwitchAggr',
*     store: 'testStore',
*     fieldName: 'switchField',
*     $set: [{ key: 'a', aggrName: outAggr1.name },
*            { key: 'b', aggrName: outAggr2.name }],
*     throwMissing: false
* });
*
* store.push({ switchField: 'a' });
* // outAggr1 prints `first`
* store.push({ switchField: 'b' });
* // outAggr2 prints `second`
* store.push({ switchField: 'b' });
* // outAggr2 prints `second`
* store.push({ switchField: 'c' });
* // nothing happens
* store.push({ switchField: 'a' });
* // outAggr1 prints `first`
*
* // clean up
* base.close();
*
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggrPageHinkley
* This stream aggregate enables detecting concept drift based on Page-Hinkley test
* on a stream of numeric data. It is based on the book
* Gamma, Knowledge Discovery from Data Streams, 2013, pp. 76
* <br>1. {@link module:qm.StreamAggr#getInteger} takes a string input (either `'drift'` or `'driftOffset'`) 
*   and returns returns the value of the property or `null` if it's unknown. `'drift'` is set to 1 if the drift 
*   has been detected in this step, otherise to 0. `'drift`' is always set to 0 in the next step. `'driftOffset'` 
*   monitors the offset (number of onStep calls) since last concept drift was detected.
* <br>2. {@link module:qm.StreamAggr#getParams} returns a parameter object.
* <br>3. {@link module:qm.StreamAggr#setParams} used for changing Page-Hinkley test parameters
*
* @property {string} [name] - The given name of the stream aggregator (autogenerated by default).
* @property {string} type - The type for the stream aggregator. <b>Important:</b> It must be equal to `'pagehinkley'`.
* @property {string} store - The name of the store consistent with the records that it processes.
* @property {string} inAggr - The name of the stream aggregate used for input (i.e. {@link module:qm.StreamAggrTimeSeriesTick}).
* @property {number} minInstances - Minimal number of instances needed for initialization of the aggregator (when can first concept drift be initialized?).
* @property {number} delta - The delta factor for the Page Hinkley test.
* @property {number} lambda - The change detection threshold.
* @property {number} alpha - The forgetting factor, used to weight the observed value and the mean.

* @example
* // main library
* let qm = require('qminer');
*
* // create a base with a simple store
* // the store records results of clustering
* base = new qm.Base({
*     mode: "createClean",
*     schema: [{
*         name: "Store",
*         fields: [
*             { name: "Value", type: "float" },
*             { name: "Time", type: "datetime" }
*         ]
*     }]
* });
*
* // create a new time series stream aggregator for the 'Store' store that takes the recorded cluster id
* // and the timestamp from the 'Time' field. The size of the window is 2 hours.
* let timeser = {
*     name: 'seriesTick1',
*     type: 'timeSeriesTick',
*     store: 'Store',
*     timestamp: 'Time',
*     value: 'Value'
* };
* let timeSeries1 = base.store("Store").addStreamAggr(timeser);
* // add a histogram aggregator, that is connected with the 'TimeSeries1' aggregator
* let aggrPHT = {
*     name: 'PageHinkley',
*     type: 'pagehinkley',
*     store: 'Store',
*     inAggr: 'seriesTick1',
*     minInstances: 1,
*     delta: 0.005,
*     lambda: 50,
*     alpha: 0.9999
* };
* store = base.store("Store");
* pht = store.addStreamAggr(aggrPHT);
*
* // creating start time
* let time = new Date();
* let changes = 0;
*
* // simulating concept drift at element 1000 in a time series
* for (let i = 0; i < 2000; i++) {
*     // add one second to the timestamp and create an ISO string
*     time.setSeconds(time.getSeconds() + 1);
*     let timeStr = time.toISOString();
*     // create value
*     let value = Math.random() * 1;
*     if (i > 1000) {
*         value = Math.random() * 2 + 1;
*     }
*     // adding values to the signal store
*     store.push({ Time: timeStr, Value: value });
*     // counting changes
*     if (pht.saveJson().drift == 1) {
*         changes++;
*     }
* }
*
* // checking if drift has been correctly detected
* if (changes >= 1) { console.log("Last concept drift was detected " + pht.val.driftOffset + " samples ago."); }
* else { console.log("No concept drift was detected!"); }
*
* // clean up
* base.close();
*
*/

class TNodeJsStreamAggr : public node::ObjectWrap {
    friend class TNodeJsUtil;
private:
    // Node framework
    static v8::Persistent<v8::Function> Constructor;
    ~TNodeJsStreamAggr() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; }
public:
    // Node framework
    static void Init(v8::Local<v8::Object> Exports);
    static const TStr GetClassId() { return "StreamAggr"; }

    // C++ wrapped object
    TWPt<TQm::TStreamAggr> SA;

    // C++ constructors
    TNodeJsStreamAggr() { }
    TNodeJsStreamAggr(TWPt<TQm::TStreamAggr> _SA) : SA(_SA) { }

    static TNodeJsStreamAggr* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);
public:
    /**
    * Resets the state of the aggregate.
    * @returns {module:qm.StreamAggr} Self. The state has been reset.
    * @example
    * // import the qm module
    * var qm = require('qminer');
    * // create a base with a simple store
    * var base = new qm.Base({
    *    mode: "createClean",
    *    schema: [
    *    {
    *        name: "Heat",
    *        fields: [
    *            { name: "Celsius", type: "float" },
    *            { name: "Time", type: "datetime" }
    *        ]
    *    }]
    * });
    *
    * // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'Celsius' field
    * // and the timestamp from the 'Time' field. The size of the window should be 1 day.
    * var timeser = {
    *    name: 'TimeSeriesAggr',
    *    type: 'timeSeriesWinBuf',
    *    store: 'Heat',
    *    timestamp: 'Time',
    *    value: 'Celsius',
    *    winsize: 86400000
    * };
    * var timeSeries = base.store("Heat").addStreamAggr(timeser);
    *
    * // add a moving average aggregator, that is connected with the 'TimeSeriesAggr' aggregator
    * var ma = {
    *    name: 'movingAverageAggr',
    *    type: 'ma',
    *    store: 'Heat',
    *    inAggr: 'TimeSeriesAggr'
    * };
    * var movingAverage = base.store("Heat").addStreamAggr(ma);
    * // reset the moving average aggregate
    * movingAverage.reset();
    * base.close();
    */
    //# exports.StreamAggr.prototype.reset = function () { return Object.create(require('qminer').StreamAggr.prototype);  };
    JsDeclareFunction(reset);

    /**
    * Executes the function that updates the aggregate. For use example see {@link module:qm.StreamAggr} constructor example.
    * @param {module:qm.StreamAggr} [Caller] - Caller stream aggregate.
    * @returns {module:qm.StreamAggr} Self. Values in the stream aggregator are changed as defined in the inner onTime function.
    */
    //# exports.StreamAggr.prototype.onStep = function () { return Object.create(require('qminer').StreamAggr.prototype); };
    JsDeclareFunction(onStep);

    /**
    * Executes the function that updates the aggregate at a given timestamp. For use example see {@link module:qm.StreamAggr} constructor example.
    * @param {TmMsec} ts - Timestamp in milliseconds.
    * @param {module:qm.StreamAggr} [Caller] - Caller stream aggregate.
    * @returns {module:qm.StreamAggr} Self. Values in the stream aggregator are changed as defined in the inner onTime function.
    */
    //# exports.StreamAggr.prototype.onTime = function (ts) { return Object.create(require('qminer').StreamAggr.prototype); };
    JsDeclareFunction(onTime);

    /**
    * Executes the function when a new record is put in store. For use example see {@link module:qm.StreamAggr} constructor example.
    * @param {module:qm.Record} rec - The record given to the stream aggregator.
    * @param {module:qm.StreamAggr} [Caller] - Caller stream aggregate.
    * @returns {module:qm.StreamAggr} Self. Values in the stream aggregator are changed as defined in the inner onAdd function.
    */
    //# exports.StreamAggr.prototype.onAdd = function (rec) { return Object.create(require('qminer').StreamAggr.prototype); };
    JsDeclareFunction(onAdd);

    /**
    * Executes the function when a record in the store is updated. For use example see {@link module:qm.StreamAggr} constructor example.
    * @param {module:qmRecord} rec - The updated record given to the stream aggregator.
    * @param {module:qm.StreamAggr} [Caller] - Caller stream aggregate.
    * @returns {module:qm.StreamAggr} Self. Values in the stream aggregator are changed as defined in the inner onUpdate function.
    */
    //# exports.StreamAggr.prototype.onUpdate = function (rec) { return Object.create(require('qminer').StreamAggr.prototype); };
    JsDeclareFunction(onUpdate);

    /**
    * Executes the function when a record in the store is deleted. For use example see {@link module:qm.StreamAggr} constructor example.
    * @param {module:qm.Record} rec - The deleted record given to the stream aggregator.
    * @param {module:qm.StreamAggr} [Caller] - Caller stream aggregate.
    * @returns {module:qm.StreamAggr} Self. The values in the stream aggregator are changed as defined in the inner onDelete function.
    */
    //# exports.StreamAggr.prototype.onDelete = function (rec) { return Object.create(require('qminer').StreamAggr.prototype); };
    JsDeclareFunction(onDelete);


    /**
    * When executed it return a JSON object as defined by the user. For use example see {@link module:qm.StreamAggr} constructor example.
    * @param {number} [limit] - The meaning is specific to each type of stream aggregator.
    * @returns {Object} A JSON object as defined by the user.
    */
    //# exports.StreamAggr.prototype.saveJson = function (limit) { return {}; };
    JsDeclareFunction(saveJson);

    /**
    * Saves the current state of the stream aggregator.
    * @param {module:fs.FOut} fout - The output stream.
    * @returns {module:fs.FOut} The output stream `fout`.
    */
    //# exports.StreamAggr.prototype.save = function (fout) { return Object.create(require('qminer').fs.FOut.prototype); }
    JsDeclareFunction(save);

    /**
    * Loads the stream aggregator, that has been previously saved.
    * @param {module:fs.FIn} fin - The input stream.
    * @returns {module:qm.StreamAggr} Self.
    */
    //# exports.StreamAggr.prototype.load = function (fin) { return Object.create(require('qminer').StreamAggr.prototype); }
    JsDeclareFunction(load);

    /**
    * Returns the current state of the stream aggregate as a json.
    * @returns {Object} JSON that represents the state.
    */
    //# exports.StreamAggr.prototype.saveStateJson = function () { return {}; }
    JsDeclareFunction(saveStateJson);

    /**
    * Loads the stream aggregator from the state.
    * @param {Object} state - The state.
    * @returns {module:qm.StreamAggr} Self.
    */
    //# exports.StreamAggr.prototype.loadStateJson = function (state) { return Object.create(require('qminer').StreamAggr.prototype); }
    JsDeclareFunction(loadStateJson);

    /**
     * Returns all the parameters of the stream aggregate
     *
     * @returns {Object} parameters
     */
    //# exports.StreamAggr.prototype.getParams = function () { return {}; }
    JsDeclareFunction(getParams);

    /**
     * Sets one or more parameters.
     *
     * @param {Object} params - JSON representation of the parameters
     */
    //# exports.StreamAggr.prototype.setParams = function (val) {}
    JsDeclareFunction(setParams);

    /**
    * A map from strings to integers
    * @param {string} [str] - The string.
    * @returns {(number | null)} A number (stream aggregator specific), possibly null if `str` was provided.
    */
    //# exports.StreamAggr.prototype.getInteger = function (str) { return 0; };
    JsDeclareFunction(getInteger);

    /**
    * Returns the value of the specific stream aggregator. For return values see {@link module:qm~StreamAggregator}.
    * @param {string} [str] - The string.
    * @returns {(number | null)} A number (stream aggregator specific), possibly null if `str` was provided.
    * @example
    * // import qm module
    * var qm = require('qminer');
    * // create a simple base containing one store
    * var base = new qm.Base({
    *    mode: 'createClean',
    *    schema: [{
    *        name: 'Grades',
    *        fields: [
    *            { name: 'Grade', type: 'int' },
    *            { name: 'Procents', type: 'float' },
    *            { name: 'Time', type: 'datetime' }
    *        ]
    *    }]
    * });
    * // create a new time series stream aggregator which takes the values from the 'Procents' field
    * // and the timestamp from the 'Time' field. The size of the window is 1 year.
    * var ts = {
    *    name: 'GradesAggr',
    *    type: 'timeSeriesWinBuf',
    *    store: 'Grades',
    *    timestamp: 'Time',
    *    value: 'Procents',
    *    winsize: 31536000000
    * };
    * var timeSeries = base.store('Grades').addStreamAggr(ts);
    * // create a new moving average stream aggregator that takes the values from the
    * // 'GradesAggr' stream aggregator
    * var ma = {
    *    name: 'AverageGrade',
    *    type: 'ma',
    *    store: 'Grades',
    *    inAggr: 'GradesAggr'
    * }
    * var averageGrade = base.store('Grades').addStreamAggr(ma);
    * // add some grades in the 'Grades' store
    * base.store("Grades").push({ Grade: 7, Procents: 65, Time: '2014-11-23T10:00:00.0' });
    * base.store("Grades").push({ Grade: 9, Procents: 88, Time: '2014-12-20T12:00:00.0' });
    * base.store("Grades").push({ Grade: 8, Procents: 70, Time: '2015-02-03T10:00:00.0' });
    * // get the average grade procents by using the getFloat method
    * var average = averageGrade.getFloat(); // returns 74 + 1/3
    * base.close();
    */
    //# exports.StreamAggr.prototype.getFloat = function (str) { return 0; };
    JsDeclareFunction(getFloat);

    /**
    * Returns the timestamp value of the newest record in buffer.
    * @returns {number} The timestamp of the newest record. It represents the number of miliseconds between the record time and 01.01.1601 time: 00:00:00.0.
    * @example
    * // import qm module
    * var qm = require('qminer');
    * // create a simple base containing one store
    * var base = new qm.Base({
    *    mode: 'createClean',
    *    schema: [{
    *        name: 'GameCollection',
    *        fields: [
    *            { name: 'GameName', type: 'string' },
    *            { name: 'Price', type: 'float' },
    *            { name: 'ReleaseDate', type: 'datetime' }
    *        ]
    *    }]
    * });
    * // create a new time series stream aggregator which takes the values from the 'Price' field
    * // and the timestamps from the 'ReleaseDate' field. The window size should be 1 month.
    * var ts = {
    *    name: 'GameSeries',
    *    type: 'timeSeriesWinBuf',
    *    store: 'GameCollection',
    *    timestamp: 'ReleaseDate',
    *    value: 'Price',
    *    winsize: 2678400000
    * };
    * var timeSeries = base.store('GameCollection').addStreamAggr(ts);
    * // create a new sum stream aggregator
    * var sum = {
    *    name: 'SumPrice',
    *    type: 'winBufSum',
    *    store: 'GameCollection',
    *    inAggr: 'GameSeries'
    * };
    * var priceSum = base.store('GameCollection').addStreamAggr(sum);
    * // put some records in the store
    * base.store('GameCollection').push({ GameName: 'Tetris', Price: 0, ReleaseDate: '1984-06-06T00:00:00.0' });
    * base.store('GameCollection').push({ GameName: 'Super Mario Bros.', Price: 100, ReleaseDate: '1985-09-13T00:00:00.0' });
    * base.store('GameCollection').push({ GameName: 'The Legend of Zelda', Price: 90, ReleaseDate: '1986-02-21T00:00:00.0 '});
    * // get the timestamp of the last bought game by using getTimestamp
    * var date = priceSum.getTimestamp(); // returns 12153801600000 (the miliseconds since 1601-01-01T00:00:00.0)
    * base.close();
    */
    //# exports.StreamAggr.prototype.getTimestamp = function () { return 0; };
    JsDeclareFunction(getTimestamp);

    /**
    * Gets the length of the vector containing the values of the stream aggregator.
    * @returns {number} The length of the vector.
    * @example
    * // import qm module
    * var qm = require('qminer');
    * // create a simple base containing one store
    * var base = new qm.Base({
    *    mode: 'createClean',
    *    schema: [{
    *        name: 'IceCreams',
    *        fields: [
    *            { name: 'Type', type: 'string' },
    *            { name: 'Price', type: 'float' },
    *            { name: 'TimeOfConsumption', type: 'datetime' }
    *        ]
    *    }]
    * });
    * // create a time series stream aggregator, that takes values from the 'Price' field and the timestamp
    * //  from the 'TimeOfConsumation' field of 'IceCream' store. The window size should be 1 day.
    * var ts = {
    *    name: 'IcePrice',
    *    type: 'timeSeriesWinBuf',
    *    store: 'IceCreams',
    *    timestamp: 'TimeOfConsumption',
    *    value: 'Price',
    *    winsize: 86400000
    * };
    * var icePrice = base.store('IceCreams').addStreamAggr(ts);
    * // add some ice creams in the store
    * base.store('IceCreams').push({ Type: 'Chocholate', Price: 5, TimeOfConsumption: '2015-07-21T09:00:00.0' });
    * base.store('IceCreams').push({ Type: 'Blue Sky', Price: 3, TimeOfConsumption: '2015-07-21T14:13:00.0' });
    * base.store('IceCreams').push({ Type: 'Stracciatella', Price: 5, TimeOfConsumption: '2015-07-21T21:05:00.0' });
    * // get the number of ice creams consumed by using getFloatLength method
    * var numberOfIceCreamsEaten = icePrice.getFloatLength(); // returns 3
    * base.close();
    */
    //# exports.StreamAggr.prototype.getFloatLength = function () { return 0; };
    JsDeclareFunction(getFloatLength);

    /**
    * Returns the value of the vector containing the values of the stream aggregator at a specific index.
    * @param {number} idx - The index.
    * @returns {number} The value of the float vector at position `idx`.
    * @example
    * // import qm module
    * var qm = require('qminer');
    * // create a simple base containing one store
    * var base = new qm.Base({
    *    mode: 'createClean',
    *    schema: [{
    *        name: 'MusicSale',
    *        fields: [
    *            { name: 'NumberOfAlbums', type: 'float' },
    *            { name: 'Time', type: 'datetime' }
    *        ]
    *    }]
    * });
    * // create a time series containing the 'NumberOfAlbums' values and getting the timestamp from the 'Time' field.
    * // The window size should be 1 week.
    * var ts = {
    *    name: 'Sales',
    *    type: 'timeSeriesWinBuf',
    *    store: 'MusicSale',
    *    timestamp: 'Time',
    *    value: 'NumberOfAlbums',
    *    winsize: 604800000
    * };
    * var weekSales = base.store('MusicSale').addStreamAggr(ts);
    * // add some records in the store
    * base.store('MusicSale').push({ NumberOfAlbums: 10, Time: '2015-03-15T00:00:00.0' });
    * base.store('MusicSale').push({ NumberOfAlbums: 15, Time: '2015-03-18T00:00:00.0' });
    * base.store('MusicSale').push({ NumberOfAlbums: 30, Time: '2015-03-19T00:00:00.0' });
    * base.store('MusicSale').push({ NumberOfAlbums: 45, Time: '2015-03-20T00:00:00.0' });
    * // get the second value of the value vector
    * var albums = weekSales.getFloatAt(1); // returns 15
    * base.close();
    */
    //# exports.StreamAggr.prototype.getFloatAt = function (idx) { return 0; };
    JsDeclareFunction(getFloatAt);

    /**
    * Gets the whole vector of values of the stream aggregator.
    * @returns {module:la.Vector} The vector containing the values of the buffer.
    * @example
    * // import qm module
    * var qm = require('qminer');
    * // create a simple base containing one store
    * var base = new qm.Base({
    *    mode: 'createClean',
    *    schema: [{
    *        name: 'Hospital',
    *        fields: [
    *            { name: 'NumberOfPatients', type: 'float' },
    *            { name: 'Date', type: 'datetime' }
    *        ]
    *    }]
    * });
    * // create a new time series stream aggregator that takes the values from the 'NumberOfPatients' field
    * // and the timestamp from the 'Date' field. The window size should be 1 week.
    * var ts = {
    *    name: 'WeekPatients',
    *    type: 'timeSeriesWinBuf',
    *    store: 'Hospital',
    *    timestamp: 'Date',
    *    value: 'NumberOfPatients',
    *    winsize: 604800000
    * };
    * var weekPatients = base.store('Hospital').addStreamAggr(ts);
    * // add some records in the store
    * base.store('Hospital').push({ NumberOfPatients: 50, Date: '2015-05-20T00:00:00.0' });
    * base.store('Hospital').push({ NumberOfPatients: 56, Date: '2015-05-21T00:00:00.0' });
    * base.store('Hospital').push({ NumberOfPatients: 120, Date: '2015-05-22T00:00:00.0' });
    * base.store('Hospital').push({ NumberOfPatients: 40, Date: '2015-05-23T00:00:00.0' });
    * // get the values that are in the time series window buffer as a vector
    * var values = weekPatients.getFloatVector(); // returns the vector [50, 56, 120, 40]
    * base.close();
    */
    //# exports.StreamAggr.prototype.getFloatVector = function () { return Object.create(require('qminer').la.Vector.prototype); };
    JsDeclareFunction(getFloatVector);

    /**
    * Gets the length of the timestamp vector of the stream aggregator.
    * @returns {number} The length of the timestamp vector.
    * @example
    * // import qm module
    * var qm = require('qminer');
    * // create a simple base containing one store
    * var base = new qm.Base({
    *    mode: 'createClean',
    *    schema: [{
    *        name: 'Medicine',
    *        fields: [
    *            { name: 'NumberOfPills', type: 'float' },
    *            { name: 'Time', type: 'datetime' }
    *        ]
    *    }]
    * });
    * // create a time series stream aggregator that takes the values from the 'NumberOfPills' field
    * // and the timestamp from the 'Time' field. The window size should be 1 week.
    * var ts = {
    *    name: 'WeekPills',
    *    type: 'timeSeriesWinBuf',
    *    store: 'Medicine',
    *    timestamp: 'Time',
    *    value: 'NumberOfPills',
    *    winsize: 604800000
    * };
    * var weekly = base.store('Medicine').addStreamAggr(ts);
    * // add some records in the store
    * base.store('Medicine').push({ NumberOfPills: 4, Time: '2015-07-21T09:00:00.0' });
    * base.store('Medicine').push({ NumberOfPills: 5, Time: '2015-07-21T19:00:00.0' });
    * base.store('Medicine').push({ NumberOfPills: 4, Time: '2015-07-22T09:00:00.0' });
    * base.store('Medicine').push({ NumberOfPills: 5, Time: '2015-07-22T19:00:00.0' });
    * base.store('Medicine').push({ NumberOfPills: 4, Time: '2015-07-23T09:00:00.0' });
    * base.store('Medicine').push({ NumberOfPills: 6, Time: '2015-07-23T19:00:00.0' });
    * base.store('Medicine').push({ NumberOfPills: 4, Time: '2015-07-24T09:00:00.0' });
    * // get the length of the timestamp vector
    * var length = weekly.getTimestampLength(); // returns 7
    * base.close();
    */
    //# exports.StreamAggr.prototype.getTimestampLength = function () { return 0; };
    JsDeclareFunction(getTimestampLength);

    /**
    * Gets the timestamp from the timestamp vector of the stream aggregator at the specific index.
    * @param {number} idx - The index.
    * @returns {number} The timestamp of the timestamp vector at position `idx`.
    * @example
    * // import qm module
    * var qm = require('qminer');
    * // create a simple base containing one store
    * var base = new qm.Base({
    *    mode: 'createClean',
    *    schema: [{
    *        name: 'Route66',
    *        fields: [
    *            { name: 'NumberOfCars', type: 'float' },
    *            { name: 'Time', type: 'datetime' }
    *        ]
    *    }]
    * });
    * // create a time series stream aggregator that takes the values from the 'NumberOfCars' field
    * // and the timestamps from the 'Time' field. The window size should be 1 day.
    * var ts = {
    *    name: 'Traffic',
    *    type: 'timeSeriesWinBuf',
    *    store: 'Route66',
    *    timestamp: 'Time',
    *    value: 'NumberOfCars',
    *    winsize: 86400000
    * };
    * var traffic = base.store('Route66').addStreamAggr(ts);
    * // add some records in the store
    * base.store('Route66').push({ NumberOfCars: 100, Time: '2015-06-15T06:00:00.0' });
    * base.store('Route66').push({ NumberOfCars: 88, Time: '2015-06-15T:10:00.0' });
    * base.store('Route66').push({ NumberOfCars: 60, Time: '2015-06-15T13:00:00.0' });
    * base.store('Route66').push({ NumberOfCars: 90, Time: '2015-06-15T18:00:00.0' });
    * base.store('Route66').push({ NumberOfCars: 110, Time: '2015-06-16T00:00:00.0' });
    * // get the third timestamp in the buffer
    * var time = traffic.getTimestampAt(2); // returns 13078864800000
    * base.close();
    */
    //# exports.StreamAggr.prototype.getTimestampAt = function (idx) { return 0; };
    JsDeclareFunction(getTimestampAt);

    /**
    * Gets the vector containing the timestamps of the stream aggregator.
    * @returns {module:la.Vector} The vector containing the timestamps.
    * @example
    * // import qm module
    * var qm = require('qminer');
    * // create a simple base containing one store
    * var base = new qm.Base({
    *    mode: 'createClean',
    *    schema: [{
    *        name: 'Signals',
    *        fields: [
    *            { name: 'BeepLoudness', type: 'float' },
    *            { name: 'Time', type: 'datetime' }
    *        ]
    *    }]
    * });
    * // create a time series stream aggregator that gets the values from the 'BeepLoudness' field and
    * // the timestamp from the 'Time' field. The window size should be 10 seconds.
    * var ts = {
    *    name: 'SignalBeep',
    *    type: 'timeSeriesWinBuf',
    *    store: 'Signals',
    *    timestamp: 'Time',
    *    value: 'BeepLoudness',
    *    winsize: 10000
    * };
    * var signalBeep = base.store('Signals').addStreamAggr(ts);
    * // add some records to the store
    * base.store('Signals').push({ BeepLoudness: 10, Time: '2015-07-21T12:30:30.0' });
    * base.store('Signals').push({ BeepLoudness: 25, Time: '2015-07-21T12:30:31.0' });
    * base.store('Signals').push({ BeepLoudness: 20, Time: '2015-07-21T12:30:32.0' });
    * // get the timestamp vector of signalBeep
    * var vec = signalBeep.getTimestampVector(); // returns vector [13081955430000, 13081955431000, 13081955432000]
    * base.close();
    */
    //# exports.StreamAggr.prototype.getTimestampVector = function () { return Object.create(require('qminer').la.Vector.prototype); };
    JsDeclareFunction(getTimestampVector);

    /**
    * Gets a vector containing the values that are entering the stream aggregator.
    * @returns {module:la.Vector} The vector containing the values that are entering the buffer.
    * @example
    * // import qm module
    * var qm = require('qminer');
    * // create a simple base containing one store
    * var base = new qm.Base({
    *    mode: 'createClean',
    *    schema: [{
    *        name: 'Noise',
    *        fields: [
    *            { name: 'Decibels', type: 'float' },
    *            { name: 'Time', type: 'datetime' }
    *        ]
    *    }]
    * });
    * // create a time series stream aggregator that takes the values from the 'Decibels' field
    * // and timestamps from the 'Time' fields. The window size should be 1 second.
    * var ts = {
    *    name: 'Music',
    *    type: 'timeSeriesWinBuf',
    *    store: 'Noise',
    *    timestamp: 'Time',
    *    value: 'Decibels',
    *    winsize: 1000
    * };
    * var music = base.store('Noise').addStreamAggr(ts);
    * // add some records in the store
    * base.store('Noise').push({ Decibels: 54, Time: '2015-07-21T14:43:00.0' });
    * base.store('Noise').push({ Decibels: 55, Time: '2015-07-21T14:43:00.200' });
    * base.store('Noise').push({ Decibels: 53, Time: '2015-07-21T14:43:00.800' });
    * base.store('Noise').push({ Decibels: 54, Time: '2015-07-21T14:43:01.0' });
    * base.store('Noise').push({ Decibels: 53, Time: '2015-07-21T14:43:01.2' });
    * // get the in vector
    * var vec = music.getInFloatVector();
    * base.close();
    */
    //# exports.StreamAggr.prototype.getInFloatVector = function () { return Object.create(require('qminer').la.Vector.prototype); };
    JsDeclareFunction(getInFloatVector);

    /**
    * Gets a vector containing the timestamps that are entering the stream aggregator.
    * @returns {module:la.Vector} The vector containing the timestamps that are entering the buffer.
    * @example
    * // import qm module
    * var qm = require('qminer');
    * // create a simple base containing one store
    * var base = new qm.Base({
    *    mode: 'createClean',
    *    schema: [{
    *        name: 'Noise',
    *        fields: [
    *            { name: 'Decibels', type: 'float' },
    *            { name: 'Time', type: 'datetime' }
    *        ]
    *    }]
    * });
    * // create a time series stream aggregator that takes the values from the 'Decibels' field
    * // and timestamps from the 'Time' fields. The window size should be 1 second.
    * var ts = {
    *    name: 'Music',
    *    type: 'timeSeriesWinBuf',
    *    store: 'Noise',
    *    timestamp: 'Time',
    *    value: 'Decibels',
    *    winsize: 1000
    * };
    * var music = base.store('Noise').addStreamAggr(ts);
    * // add some records in the store
    * base.store('Noise').push({ Decibels: 54, Time: '2015-07-21T14:43:00.0' });
    * base.store('Noise').push({ Decibels: 55, Time: '2015-07-21T14:43:00.200' });
    * base.store('Noise').push({ Decibels: 53, Time: '2015-07-21T14:43:00.800' });
    * base.store('Noise').push({ Decibels: 54, Time: '2015-07-21T14:43:01.0' });
    * base.store('Noise').push({ Decibels: 53, Time: '2015-07-21T14:43:01.2' });
    * // get the in timestamps
    * var vec = music.getInTimestampVector();
    * base.close();
    */
    //# exports.StreamAggr.prototype.getInTimestampVector = function () { return Object.create(require('qminer').la.Vector.prototype); };
    JsDeclareFunction(getInTimestampVector);

    /**
    * Gets a vector containing the values that are leaving the stream aggregator.
    * @returns {module:la.Vector} The vector containing the values that are leaving the buffer.
    * @example
    * // import qm module
    * var qm = require('qminer');
    * // create a simple base containing one store
    * var base = new qm.Base({
    *    mode: 'createClean',
    *    schema: [{
    *        name: 'F1',
    *        fields: [
    *            { name: 'Driver', type: 'string' },
    *            { name: 'Speed', type: 'float' },
    *            { name: 'Time', type: 'datetime' }
    *        ]
    *    }]
    * });
    * // create a time series stream aggregator that gets the values from the 'Speed' field
    * // and the timestamp from the 'Time' field. The window size should be 5 minutes.
    * var ts = {
    *    name: 'Sensor',
    *    type: 'timeSeriesWinBuf',
    *    store: 'F1',
    *    timestamp: 'Time',
    *    value: 'Speed',
    *    winsize: 300000
    * };
    * var sensor = base.store('F1').addStreamAggr(ts);
    * // add some records to the store
    * base.store('F1').push({ Driver: 'Sebastian Vettel', Speed: 203.4, Time: '2015-07-19T09:32:01.0' });
    * base.store('F1').push({ Driver: 'Thomas "Tommy" Angelo', Speed: 152.8, Time: '2015-07-19T09:35:23.0' });
    * base.store('F1').push({ Driver: 'Mark Ham', Speed: 189.5, Time: '2015-07-19T09:38:43.0' });
    * base.store('F1').push({ Driver: 'Speedy Gonzales', Speed: 171.4, Time: '2015-07-19T09:40:32.0' });
    * // get the values, that have got out of the window buffer.
    * // because the window size is 5 seconds, the last value that have left the buffer is 152.8
    * var left = sensor.getOutFloatVector(); // returns [152.8]
    * base.close();
    */
    //# exports.StreamAggr.prototype.getOutFloatVector = function () { return Object.create(require('qminer').la.Vector.prototype); };
    JsDeclareFunction(getOutFloatVector);

    /**
    * Gets a vector containing the timestamps that are leaving the stream aggregator.
    * @returns {module:la.Vector} The vector containing the leaving timestamps.
    * @example
    * // import qm module
    * var qm = require('qminer');
    * // create a simple base containing one store
    * var base = new qm.Base({
    *    mode: 'createClean',
    *    schema: [{
    *        name: 'Noise',
    *        fields: [
    *            { name: 'Decibels', type: 'float' },
    *            { name: 'Time', type: 'datetime' }
    *        ]
    *    }]
    * });
    * // create a time series stream aggregator that takes the values from the 'Decibels' field
    * // and timestamps from the 'Time' fields. The window size should be 1 second.
    * var ts = {
    *    name: 'Music',
    *    type: 'timeSeriesWinBuf',
    *    store: 'Noise',
    *    timestamp: 'Time',
    *    value: 'Decibels',
    *    winsize: 1000
    * };
    * var music = base.store('Noise').addStreamAggr(ts);
    * // add some records in the store
    * base.store('Noise').push({ Decibels: 54, Time: '2015-07-21T14:43:00.0' });
    * base.store('Noise').push({ Decibels: 55, Time: '2015-07-21T14:43:00.200' });
    * base.store('Noise').push({ Decibels: 54, Time: '2015-07-21T14:43:00.400' });
    * base.store('Noise').push({ Decibels: 53, Time: '2015-07-21T14:43:00.600' });
    * base.store('Noise').push({ Decibels: 53, Time: '2015-07-21T14:43:00.800' });
    * base.store('Noise').push({ Decibels: 54, Time: '2015-07-21T14:43:01.0' });
    * base.store('Noise').push({ Decibels: 53, Time: '2015-07-21T14:43:01.2' });
    * // get the timestamps that just left the window buffer by adding the last record
    * var last = music.getOutTimestampVector(); // returns [13081963380000]
    * base.close();
    */
    //# exports.StreamAggr.prototype.getOutTimestampVector = function () { return Object.create(require('qminer').la.Vector.prototype); };
    JsDeclareFunction(getOutTimestampVector);

    /**
    * Gets the number of records in the stream aggregator.
    * @returns {number} The number of records in the buffer.
    * @example
    * // import qm module
    * var qm = require('qminer');
    * // create a simple base containing one store
    * var base = new qm.Base({
    *    mode: 'createClean',
    *    schema: [{
    *        name: 'MusicSale',
    *        fields: [
    *            { name: 'NumberOfAlbums', type: 'float' },
    *            { name: 'Time', type: 'datetime' }
    *        ]
    *    }]
    * });
    * // create a time series containing the values from the 'NumberOfAlbums' field and
    * // the timestamp from the 'Time' field. The window size should be 1 week.
    * var ts = {
    *    name: 'Sales',
    *    type: 'timeSeriesWinBuf',
    *    store: 'MusicSale',
    *    timestamp: 'Time',
    *    value: 'NumberOfAlbums',
    *    winsize: 604800000
    * };
    * var weekSales = base.store('MusicSale').addStreamAggr(ts);
    * // add some records in the store
    * base.store('MusicSale').push({ NumberOfAlbums: 10, Time: '2015-03-15T00:00:00.0' });
    * base.store('MusicSale').push({ NumberOfAlbums: 15, Time: '2015-03-18T00:00:00.0' });
    * base.store('MusicSale').push({ NumberOfAlbums: 30, Time: '2015-03-19T00:00:00.0' });
    * base.store('MusicSale').push({ NumberOfAlbums: 45, Time: '2015-03-20T00:00:00.0' });
    * // get the number of records in the window buffer
    * var num = weekSales.getNumberOfRecords(); // returns 4
    * base.close();
    */
    //# exports.StreamAggr.prototype.getNumberOfRecords = function () { return 0; };
    JsDeclareFunction(getNumberOfRecords);

    // IValTmIO
    /**
    * Gets the vector of 'just-in' values (values that have just entered the buffer). Values can be floats or sparse vectors.
    * @returns {(module:la.Vector | module:la.SparseMatrix)} The vector or sparse matrix of values that just entered the buffer.
    * @example
    * // import qm module
    * var qm = require('qminer');
    * // create a simple base containing one store
    * var base = new qm.Base({
    *      mode: 'createClean',
    *      schema: [{
    *        name: 'Docs',
    *        fields: [
    *          { name: 'Time', type: 'datetime' },
    *          { name: 'Text', type: 'string' }
    *        ]
    *      }]
    * });
    * store = base.store('Docs');
    * // the following aggregate maintains a window buffer (1000 milliseconds) with no delay
    * // and contains a categorical feature extractor. The extractor maps records in the buffer
    * // to sparse vectors (indicator vectors for growing set of categories). Each record that
    * // enters the buffer updates the feature extractor (potentially introduces a new category,
    * // which increases the dimensionality).
    * var aggr = {
    *      type: 'timeSeriesWinBufFeatureSpace',
    *      store: 'Docs',
    *      timestamp: 'Time',
    *      featureSpace: {
    *        type: "categorical",
    *        source: "Docs",
    *        field: "Text"
    *      },
    *      winsize: 1000
    * };
    * var streamAggregate = store.addStreamAggr(aggr);
    * store.push({ Time: '2015-06-10T14:13:32.0', Text: 'a' });
    * store.push({ Time: '2015-06-10T14:13:33.0', Text: 'b' });
    * store.push({ Time: '2015-06-10T14:13:34.0', Text: 'c' });
    * // we have three dimensions, where "a" -> [1,0,0], "b" -> [0,1,0], "c" -> [0,0,1]
    * // the first record just fell out of the buffer, the third record just entered the buffer
    * // and buffer currently contains the second and the third record.
    * // In case of the feature space based window buffer, the vectors of just-in, just-out and in-the-buffer
    * // values correspond to vectors of sparse vectors = sparse matrices.
    * streamAggregate.getInValueVector().print(); // one column, one nonzero element at index 2
    * // = [
    * // 2 0 1.000000
    * // ]
    * streamAggregate.getOutValueVector().print(); // one column, one nonzero element at index 0
    * // = [
    * // 0 0 1.000000
    * // ]
    * streamAggregate.getValueVector().print(); // two column vectors, each with one nonzero element
    * // = [
    * // 1 0 1.000000
    * // 2 1 1.000000
    * // ]
    *
    * base.close();
    */
    //# exports.StreamAggr.prototype.getInValueVector = function () { };
    JsDeclareFunction(getInValueVector);

    /**
    * Gets the vector of 'just-out' values (values that have just fallen out of the buffer). Values can be floats or sparse vectors.
    * For use example see {@link module:qm.StreamAggr#getOutFloatVector} example.
    * @returns {(module:la.Vector | module:la.SparseMatrix)} Vector of floats or a vector of sparse vectors.
    */
    //# exports.StreamAggr.prototype.getOutValueVector = function () { };
    JsDeclareFunction(getOutValueVector);

    // IValVec
    /**
    * Gets the vector of values in the buffer. Values can be floats or sparse vectors. For use example see {@link module:qm.SreamAggr#getInValueVector}.
    * @returns {(module:la.Vector | module:la.SparseMatrix)} Vector of floats or a vector of sparse vectors
    */
    //# exports.StreamAggr.prototype.getValueVector = function () { };
    JsDeclareFunction(getValueVector);

    /**
    * Returns a feature space.
    * @returns {module:qm.FeatureSpace} The feature space.
    */
    //# exports.StreamAggr.prototype.getFeatureSpace = function() { return Object.create(require('qminer').FeatureSpace.prototype); };
    JsDeclareFunction(getFeatureSpace);

    /**
    * Returns the name of the stream aggregate. Type `string`.
    */
    //# exports.StreamAggr.prototype.name = "";
    JsDeclareProperty(name);

    /**
    * Returns the JSON object of the stream aggregate. Same as the method saveJson. Type `object`.
    */
    //# exports.StreamAggr.prototype.val = {};
    JsDeclareProperty(val);

    /**
    * Returns true when the stream aggregate has enough data to initialize its internal state. Type `init`.
    */
    //# exports.StreamAggr.prototype.init = false;
    JsDeclareProperty(init);
};

///////////////////////////////
// JavaScript Stream Aggregator
class TNodeJsFuncStreamAggr :
    public TQm::TStreamAggr,
    public TQm::TStreamAggrOut::IInt,
    public TQm::TStreamAggrOut::ITm,
    public TQm::TStreamAggrOut::IFlt,
    public TQm::TStreamAggrOut::ITmIO,
    public TQm::TStreamAggrOut::IFltIO,
    public TQm::TStreamAggrOut::ITmVec,
    public TQm::TStreamAggrOut::IFltVec,
    public TQm::TStreamAggrOut::INmFlt,
    public TQm::TStreamAggrOut::INmInt,
    public TQm::TStreamAggrOut::ISparseVec
{
private:
    v8::Persistent<v8::Object> ThisObj;
    // callbacks
    v8::Persistent<v8::Function> ResetFun;
    v8::Persistent<v8::Function> OnStepFun;
    v8::Persistent<v8::Function> OnTimeFun;
    v8::Persistent<v8::Function> OnAddFun;
    v8::Persistent<v8::Function> OnUpdateFun;
    v8::Persistent<v8::Function> OnDeleteFun;
    v8::Persistent<v8::Function> SaveJsonFun;
    v8::Persistent<v8::Function> IsInitFun;

    // Param get/set
    v8::Persistent<v8::Function> GetParamsFun;
    v8::Persistent<v8::Function> SetParamsFun;

    // Int
    v8::Persistent<v8::Function> GetIntFun;
    // IFlt
    v8::Persistent<v8::Function> GetFltFun;
    // ITm
    v8::Persistent<v8::Function> GetTmMSecsFun;
    // IFltIO
    v8::Persistent<v8::Function> GetInFltVFun;
    v8::Persistent<v8::Function> GetOutFltVFun;
    // ITmIO
    v8::Persistent<v8::Function> GetInTmMSecsVFun;
    v8::Persistent<v8::Function> GetOutTmMSecsVFun;
    v8::Persistent<v8::Function> GetNFun;

    // IFltVec
    v8::Persistent<v8::Function> GetFltLenFun;
    v8::Persistent<v8::Function> GetFltAtFun;
    v8::Persistent<v8::Function> GetFltVFun;
    // ITmVec
    v8::Persistent<v8::Function> GetTmLenFun;
    v8::Persistent<v8::Function> GetTmAtFun;
    v8::Persistent<v8::Function> GetTmVFun;
    // INmFlt
    v8::Persistent<v8::Function> IsNmFltFun;
    v8::Persistent<v8::Function> GetNmFltFun;

    // INmInt
    v8::Persistent<v8::Function> IsNmIntFun;
    v8::Persistent<v8::Function> GetNmIntFun;

    // Serialization
    v8::Persistent<v8::Function> SaveFun;
    v8::Persistent<v8::Function> LoadFun;
    v8::Persistent<v8::Function> SaveStateJsonFun;
    v8::Persistent<v8::Function> LoadStateJsonFun;

public:
    TNodeJsFuncStreamAggr(TWPt<TQm::TBase> _Base, const TStr& _AggrNm, v8::Local<v8::Object> TriggerVal);
    static TQm::PStreamAggr New(TWPt<TQm::TBase> _Base, const TStr& _AggrNm, v8::Local<v8::Object> TriggerVal) {
        return new TNodeJsFuncStreamAggr(_Base, _AggrNm, TriggerVal);
    }

    ~TNodeJsFuncStreamAggr();

    void Reset();
    void OnStep(const TWPt<TStreamAggr>& CallerAggr);
    void OnTime(const uint64& Time, const TWPt<TStreamAggr>& CallerAggr);
    void OnAddRec(const TQm::TRec& Rec, const TWPt<TStreamAggr>& CallerAggr);
    void OnUpdateRec(const TQm::TRec& Rec, const TWPt<TStreamAggr>& CallerAggr);
    void OnDeleteRec(const TQm::TRec& Rec, const TWPt<TStreamAggr>& CallerAggr);
    PJsonVal SaveJson(const int& Limit) const;
    bool IsInit() const;

    // stream aggregator type name
    static TStr GetType() { return "javaScript"; }
    TStr Type() const { return GetType(); }
    void SaveState(TSOut& SOut) const;
    void LoadState(TSIn& SIn);
    PJsonVal SaveStateJson() const;
    void LoadStateJson(const PJsonVal& State);

    PJsonVal GetParams() const;
    void SetParams(const PJsonVal& Params);

    // IInt
    int GetInt() const;
    // IFlt
    double GetFlt() const;
    // ITm
    uint64 GetTmMSecs() const;

    // IFltIO
    void GetInValV(TFltV& ValV) const;
    void GetOutValV(TFltV& ValV) const;
    // ITmIO
    void GetInTmMSecsV(TUInt64V& MSecsV) const;
    void GetOutTmMSecsV(TUInt64V& MSecsV) const;
    // in buffer
    int GetN() const;

    // IFltVec
    int GetVals() const;
    void GetVal(const int& ElN, TFlt& Val) const; // GetFltAtFun
    void GetValV(TFltV& ValV) const;
    // ITmVec
    int GetTmLen() const;
    uint64 GetTm(const int& ElN) const; // GetTmAtFun
    void GetTmV(TUInt64V& TmMSecsV) const;
    // INmFlt
    bool IsNmFlt(const TStr& Nm) const;
    double GetNmFlt(const TStr& Nm) const;
    // INmInt
    bool IsNmInt(const TStr& Nm) const;
    int GetNmInt(const TStr& Nm) const;
    // ISparseVec
    int GetSparseVecLen() const;
    TIntFltKd GetSparseVecVal(const int& ElN) const; // GetFltAtFun
    void GetSparseVec(TIntFltKdV& ValV) const;
};

#endif

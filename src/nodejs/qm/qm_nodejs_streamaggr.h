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
* Qminer module.
* @module qm
* @example
* // import module
* var qm = require('qminer');
*/

/**
* Stream Aggregate
* @classdesc Represents the stream aggregate. The class can construct these {@link module:qm~StreamAggregators}. Also turn to these stream aggregators to see 
* which methods are implemented for them.
* @class
* @param {module:qm.Base} base - The base object on which it's created.
* @param {(Object | function)} json - The JSON object containing the schema of the stream aggregate or the function object defining the operations of the stream aggregate.
* @param {(string | Array.<string>)} [storeName] - A store name or an array of store names, where the aggregate will be registered.
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
*    var length = 0;
*    this.name = 'nameLength',
*    this.onAdd = function (rec) {
*        length = rec.Name.length;
*    };
*    this.saveJson = function (limit) {
*        return { val: length };
*    }
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
*/
//# exports.StreamAggr = function (base, json, storeName) { return Object.create(require('qminer').StreamAggr.prototype); };

/**
* @typedef {module:qm.StreamAggr} StreamAggregators
* Stream aggregator types.
* @property {module:qm~StreamAggregateTimeSeriesWindow} timeSeries - The time series type.
* @property {module:qm~StreamAggregateCount} count - The count type.
* @property {module:qm~StreamAggregateSum} sum - The sum type.
* @property {module:qm~StreamAggregateMin} min - The minimal type.
* @property {module:qm~StreamAggregateMax} max - The maximal type.
* @property {module:qm~StreamAggregateTimeSeriesTick} tick - The time series tick type.
* @property {module:qm~StreamAggregateMovingAverage} ma - The moving average type.
* @property {module:qm~StreamAggregateEMA} ema - The exponental moving average type.
* @property {module:qm~StreamAggregateMovingVariance} var - The moving variance type.
* @property {module:qm~StreamAggregateMovingCovariance} cov - The moving covariance type.
* @property {module:qm~StreamAggregateMovingCorrelation} cor - The moving correlation type.
* @property {module:qm~StreamAggregateResampler} res - The resampler type.
* @property {module:qm~StreamAggregateMerger} mer - The merger type.
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggregateTimeSeriesWindow
* This stream aggregator represents the time series window buffer. It stores the values inside a moving window. 
* It implements all the methods of <b>except</b> {@link module:qm.StreamAggr#getFloat}, {@link module:qm.StreamAggr#getTimestamp}.
* @property {string} name - The given name of the stream aggregator.
* @property {string} type - The type of the stream aggregator. It must be equal to <b>'timeSeriesWinBuf'</b>.
* @property {string} store - The name of the store from which to takes the data.
* @property {string} timestamp - The field of the store, where it takes the timestamp.
* @property {string} value - The field of the store, where it takes the values.
* @property {number} winsize - The size of the window, in miliseconds.
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
*            { name: "Celcius", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
*
* // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'Celcius' field
* // and the timestamp from the 'Time' field. The size of the window is 2 seconds (2000ms).
* var aggr = {
*    name: 'TimeSeriesAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celcius',
*    winsize: 2 * 1000
* };
* base.store("Heat").addStreamAggr(aggr); 
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggregateCount
* This stream aggregator represents the count moving window buffer. It counts the number of records in the
* stream aggregator, that it connects to. It implements the following methods:
* <br>{@link module:qm.StreamAggr#getFloat} returns the number of records in the it's buffer window. 
* <br>{@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in it's buffer window.
* @property {string} name - The given name of the stream aggregator.
* @property {string} type - The type of the stream aggregator. It must be equal to <b>'winBufCount'</b>.
* @property {string} store - The name of the store from which it takes the data.
* @property {string} inAggr - The name of the stream aggregator to which it connects and gets the data.
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
* // create a new time series stream aggregator for the 'Students' store, that takes the values from the 'Id' field
* // and the timestamp from the 'TimeOfGraduation' field. The size of the window should be 1 month.
* var timeser = {
*    name: 'TimeSeriesAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Students',
*    timestamp: 'TimeOfGraduation',
*    value: 'Id',
*    winsize: 31 * 24 * 60 * 60 * 1000
* };
* var timeSeries = base.store("Heat").addStreamAggr(timeser);
*
* // add a count aggregator, that is connected with the 'TimeSeriesAggr' aggregator
* var co = {
*    name: 'CountAggr',
*    type: 'winBufCount',
*    store: 'Students',
*    inAggr: 'TimeSeriesAggr'
* };
* var count = base.store("Heat").addStreamAggr(co);
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggregateSum
* This stream aggregator represents the sum moving window buffer. It sums all the values, that are in the connected stream aggregator.
* It implements the following methods:
* <br>{@link module:qm.StreamAggr#getFloat} returns the sum of the values of the records in the it's buffer window.
* <br>{@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in it's buffer window.
* @property {string} name - The given name of the stream aggregator.
* @property {string} type - The type of the stream aggregator. It must be equal to <b>'winBufSum'</b>.
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
*    winsize: 7 * 24 * 60 * 60 * 1000
* };
* var timeSeries = base.store("Heat").addStreamAggr(timeser);
*
* // add a sum aggregator, that is connected with the 'TimeSeriesAggr' aggregator
* var sum = {
*    name: 'SumAggr',
*    type: 'winBufSum',
*    store: 'Heat',
*    inAggr: 'TimeSeriesAggr'
* };
* var sumAggr = base.store("Heat").addStreamAggr(sum);
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggregateMin
* This stream aggregator represents the minimum moving window buffer. It monitors the minimal value in the connected stream aggregator.
* It implements the following methods:
* <br>{@link module:qm.StreamAggr#getFloat} returns the minimal value of the records in it's buffer window.
* <br>{@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in it's buffer window.
* @property {string} name - The given name of the stream aggregator.
* @property {string} type - The type of the stream aggregator. It must be equal to <b>'winBufMin'</b>.
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
*            { name: "Celcius", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
*
* // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'Celcius' field
* // and the timestamp from the 'Time' field. The size of the window is 1 day.
* var timeser = {
*    name: 'TimeSeriesAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celcius',
*    winsize: 24 * 60 * 60 * 1000
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
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggregateMax
* This stream aggregator represents the maximum moving window buffer. It monitors the maximal value in the connected stream aggregator. 
* It implements the following methods:
* <br>{@link module:qm.StreamAggr#getFloat} returns the maximal value of the records in the it's buffer window.
* <br>{@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in it's buffer window.
* @property {string} name - The given name of the stream aggregator.
* @property {string} type - The type for the stream aggregator. It must be equal to <b>'winBufMax'</b>.
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
*            { name: "Celcius", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
*
* // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'Celcius' field
* // and the timestamp from the 'Time' field. The size of the window is 1 day.
* var timeser = {
*    name: 'TimeSeriesAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celcius',
*    winsize: 24 * 60 * 60 * 1000
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
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggregateTimeSeriesTick
* This stream aggregator represents the time series tick window buffer. It exposes the data to other stream aggregators 
* (similar to {@link module:qm~StreamAggr_TimeSeriesWindow}). It implements the following methods:
* <br>{@link module:qm.StreamAggr#getFloat} returns the last value added in it's buffer.
* <br>{@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in it's buffer.
* @property {string} name - The given name for the stream aggregator.
* @property {string} type - The type of the stream aggregator. It must be equal to <b>'timeSeriesTick'</b>.
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
* var timeSeriesTick = base.store("Heat").addStreamAggr(tick);
*/

/**
* @typedef {module:qmStreamAggr} StreamAggregateMovingAverage
* This stream aggregator represents the moving average window buffer. It calculates the moving average value of the connected stream aggregator values.
* It implements the following methods:
* <br>{@link module:qm.StreamAggr#getFloat} returns the average of the values in it's buffer window.
* <br>{@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in it's buffer window.
* @property {string} name - The given name of the stream aggregator.
* @property {string} type - The type of the stream aggregator. It must be equal to <b>'ma'</b>.
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
*            { name: "Celcius", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
*
* // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'Celcius' field
* // and the timestamp from the 'Time' field. The size of the window should be 1 day.
* var timeser = {
*    name: 'TimeSeriesAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celcius',
*    winsize: 24 * 60 * 60 * 1000
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
*/

/**
* @typedef {module:qmStreamAggr} StreamAggregateEMA
* This stream aggregator represents the exponential moving average window buffer. It calculates the weighted moving average 
* of the values in the connected stream aggregator, where the weights are exponentially decreasing.  It implements the following methods:
* <br>{@link module:qm.StreamAggr#getFloat} returns the exponentional average of the values in it's buffer window.
* <br>{@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in it's buffer window.
* @property {string} name - The given name for the stream aggregator.
* @property {string} type - The type of the stream aggregator. It must be equal to <b>'ema'</b>.
* @property {string} store - The name of the store from which it takes the data.
* @property {string} inAggr - The name of the stream aggregator to which it connects and gets data. 
* It <b>cannot</b> connect to the {@link module:qm~StreamAggr_TimeSeriesWindow}.
* @property {string} emaType - The type of interpolation. The choices are 'previous', 'linear' and 'next'.
* <br> Type 'previous' interpolates with the previous value. 
* <br> Type 'next' interpolates with the next value.
* <br> Type 'linear' makes a linear interpolation.
* @property {number} interval - The time interval defining the decay. It must be greater than initWindow.
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
*            { name: "Celcius", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
*
* // create a new time series tick stream aggregator for the 'Heat' store, that takes the values from the 'Celcius' field
* // and the timestamp from the 'Time' field. The size of the window should be 1 hour.
* var timeser = {
*    name: 'TimeSeriesAggr',
*    type: 'timeSeriesTick',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celcius',
*    winsize: 60 * 60 * 1000
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
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggregateMovingVariance
* This stream aggregator represents the moving variance window buffer. It calculates the moving variance of the stream aggregator, that it's connected to. 
* It implements the following methods:
* <br>{@link module:qm.StreamAggr#getFloat} returns the variance of the values in it's buffer window.
* <br>{@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in it's buffer window.
* @property {string} name - The given name for the stream aggregator.
* @property {string} type - The type of the stream aggregator. It must be equal to <b>'variance'</b>.
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
*            { name: "Celcius", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
*
* // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'Celcius' field
* // and the timestamp from the 'Time' field. The size of the window is 1 day.
* var timeser = {
*    name: 'TimeSeriesAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celcius',
*    winsize: 24 * 60 * 60 * 1000
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
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggregateMovingCovariance
* This stream aggregator represents the moving covariance window buffer. It calculates the moving covariance of the two stream aggregators, that it's connected to.
* It implements the following methods:
* <br>{@link module:qm.StreamAggr#getFloat} returns the covariance of the values in it's buffer window.
* <br>{@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in it's buffer window.
* @property {string} name - The given name for the stream aggregator.
* @property {string} type - The type of the stream aggregator. It must be equal to <b>'covariance'</b>.
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
*            { name: "Celcius", type: "float" },
*            { name: "WaterConsumption", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
*
* // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'Celcius' field
* // and the timestamp from the 'Time' field. The size of the window is 1 day.
* var celcius = {
*    name: 'CelciusAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celcius',
*    winsize: 24 * 60 * 60 * 1000
* }; base.store("Heat").addStreamAggr(celcius);
*
* // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'WaterConsumption' field
* // and the timestamp from the 'Time' field. The size of the window is 1 day.
* var water = {
*    name: 'WaterAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'WaterConsumption',
*    winsize: 24 * 60 * 60 * 1000
* }; base.store("Heat").addStreamAggr(water);
*
* // add a covariance aggregator, that is connected with the 'CelciusAggr' and 'WaterAggr' stream aggregators
* var covariance = {
*    name: 'covAggr',
*    type: 'covariance',
*    store: 'Heat',
*    inAggrX: 'CelciusAggr',
*    inAggrY: 'WaterAggr'
* };
* var covarianceAggr = base.store("Heat").addStreamAggr(covariance);
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggregateMovingCorrelation
* This stream aggregator represents the moving covariance window buffer. It calculates the moving correlation of the three stream aggregators,
* that it's connected to. It implements the following methods:
* <br>{@link module:qm.StreamAggr#getFloat} returns the correlation of the values in it's buffer window.
* <br>{@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in it's buffer window.
* @property {string} name - The given name for the stream aggregator.
* @property {string} type - The type of the stream aggregator. It must be equal to <b>'correlation'</b>.
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
*            { name: "Celcius", type: "float" },
*            { name: "WaterConsumption", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
*
* // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'Celcius' field
* // and the timestamp from the 'Time' field. The size of the window is 1 day.
* var celcius = {
*    name: 'CelciusAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celcius',
*    winsize: 24 * 60 * 60 * 1000
* }; base.store("Heat").addStreamAggr(celcius);
*
* // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'WaterConsumption' field
* // and the timestamp from the 'Time' field. The size of the window is 1 day.
* var water = {
*    name: 'WaterAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'WaterConsumption',
*    winsize: 24 * 60 * 60 * 1000
* }; base.store("Heat").addStreamAggr(water);
*
* // add a covariance aggregator, that is connected with the 'CelciusAggr' and 'WaterAggr' aggregators
* var covariance = {
*    name: 'covarianceAggr',
*    type: 'covariance',
*    store: 'Heat',
*    inAggrX: 'CelciusAggr',
*    inAggrY: 'WaterAggr'
* }; base.store("Heat").addStreamAggr(covariance);
*
* // add the two variance aggregators, that take from the 'Celcius' and 'WaterConsumption' fields, respectively.
* var celVar = {
*    name: 'celciusVarAggr',
*    type: 'variance',
*    store: 'Heat',
*    inAggr: 'CelciusAggr'
* }; base.store("Heat").addStreamAggr(celVar);
*
* var waterVar = {
*    name: 'waterVarAggr',
*    type: 'variance',
*    store: 'Heat',
*    inAggr: 'WaterAggr'
* }; base.store("Heat").addStreamAggr(waterVar);
*
* // add a correlation aggregator, that is connected to 'CovarianceAggr', 'CelciusVarAggr' and 'WaterValAggr' aggregators
* var corr = {
*    name: 'corrAggr',
*    type: 'correlation',
*    store: 'Heat',
*    inAggrCov: 'covarianceAggr',
*    inAggrVarX: 'celciusVarAggr',
*    inAggrVarY: 'waterVarAggr'
* };
* var correlation = base.store("Heat").addStreamAggr(corr);
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggregateResampler
* This stream aggregator represents the resampler window buffer. It creates new values that are interpolated by using the values from an existing store.
* No methods are implemented for this aggregator. 
* @property {string} name - The given name for the stream aggregator.
* @property {string} type - The type of the stream aggregator. It must be equal to <b>'resampler'</b>.
* @property {string} store - The name of the store from which it takes the data.
* @property {string} outStore - The store in which the samples are stored.
* @property {string} timestamp - The store field from which it takes the timestamps.
* @property {Object} fields - The json, which contains:
* <br> name (string) - the store field from which it takes the values.
* <br> interpolator (string) - the type of the interpolation. The options are 'previous', 'next' and 'linear'.
* <br> The 'previous' type interpolates with the previous value.
* @property {boolean} createStore - If the outStore must be created.
* @property {number} interval - The size/frequency the interpolated values should be given.
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
*            { name: "Celcius", type: "float" },
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
* // create a new resampler stream aggregator for the 'Heat' store, that takes the values from the 'Celcius' field
* // and the timestamp from the 'Time' field. The interpolated values are stored in the 'interpolatedValues' store.
* // The interpolation should be linear and the interval should be 2 seconds.
* var res = {
*    name: 'resamplerAggr',
*    type: 'resampler',
*    store: 'Heat',
*    outStore: 'interpolatedValues',
*    timestamp: 'Time',
*    fields: [{
*        name: 'Celcius',
*        interpolator: 'linear'
*    }],
*    createStore: false,
*    interval: 2 * 1000
* };
* var resampler = base.store("Heat").addStreamAggr(res);
*/

/**
* @typedef {module:qm.StreamAggr} StreamAggregateMerger
* This stream aggregator represents the merger aggregator. It merges records from two or more stores into a new store
* depending on the timestamp. No methods are implemented for this aggregator.
* <image src="pictures/merger.gif" alt="Merger Animation">
* @property {string} name - The given name for the stream aggregator.
* @property {string} type - The type of the stream aggregator. It must be equal to <b>'stmerger'</b>.
* @property {string} outStore - The name of the store where it saves the merged records.
* @property {boolean} createStore - If the outStore must be created.
* @property {string} timestamp - The store field of outStore, where the timestamp is saved.
* @property {Array.<Object>} fields - An array of json objects. The json objects contain:
* <br> source (string) - The name of the store, from which it takes the values.
* <br> inField (string) - The field name of source, from which it takes the values.
* <br> outField (string) - The field name of outStore, into which it saves the values.
* <br> interpolation (string) - The type of the interpolation. The options are: 'previous', 'next' and 'linear'.
* <br> timestamp (string) - The field name of source, where the timestamp is saved. 
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
*            { name: "Celcius", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    },
*    {
*        name: "Merged",
*        fields: [
*            { name: "NumberOfCars", type: "float" },
*            { name: "Celcius", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
* // create a new merger stream aggregator that mergers the records of the 'Cars' and 'Temperature' stores.
* // The records are interpolated linearly and stored in the 'Merged' store.
* var mer = {
*    name: 'MergerAggr',
*    type: 'stmerger',
*    outStore: 'Merged',
*    createStore: false,
*    timestamp: 'Time',
*    fields: [
*        { source: 'Cars', inField: 'NumberOfCars', outField: 'NumberOfCars', interpolation: 'linear', timestamp: 'Time' },
*        { source: 'Temperature', inField: 'Celcius', outField: 'Celcius', interpolation: 'linear', timestamp: 'Time' }
*    ]
* };
* var merger = new qm.StreamAggr(base, mer);
*/

class TNodeJsSA : public node::ObjectWrap {
private:
	// Node framework
	static v8::Persistent<v8::Function> constructor;
public:
	// Node framework
	static void Init(v8::Handle<v8::Object> exports);
	// Wrapping C++ object
	static v8::Local<v8::Object> New(TWPt<TQm::TStreamAggr> _SA);
	// C++ constructors
	TNodeJsSA() { }
	TNodeJsSA(TWPt<TQm::TStreamAggr> _SA) : SA(_SA) { }
	~TNodeJsSA() { }
	// Node framework (constructor method)
	JsDeclareFunction(New);
public:
	// C++ wrapped object
	TWPt<TQm::TStreamAggr> SA;

	//! 
	//! **Functions and properties:**
	//! 	
	//!- `sa = sa.onAdd(rec)` -- executes onAdd function given an input record `rec` and returns self
	/**
	* Executes the function when a new record is put in store.
	* @param {module:qm.Record} rec - The record given to the stream aggregator.
	* @returns {module:qm.StreamAggr} Self. Values in the stream aggregator are changed as defined in the inner onAdd function.
	*/
	//# exports.StreamAggr.prototype.onAdd = function (rec) { return Object.create(require('qminer').StreamAggr.prototype); };
	JsDeclareFunction(onAdd);

	//!- `sa = sa.onUpdate(rec)` -- executes onUpdate function given an input record `rec` and returns self
	/**
	* Executes the function when a record in the store is updated.
	* @param {module:qmRecord} rec - The updated record given to the stream aggregator.
	* @returns {module:qm.StreamAggr} Self. Values in the stream aggregator are changed as defined in the inner onUpdate function.
	*/
	//# exports.StreamAggr.prototype.onUpdate = function (rec) { return Object.create(require('qminer').StreamAggr.prototype); };
	JsDeclareFunction(onUpdate);

	//!- `sa = sa.onDelete(rec)` -- executes onDelete function given an input record `rec` and returns self
	/**
	* Executes the function when a record in the store is deleted.
	* @param {module:qm.Record} rec - The deleted record given to the stream aggregator.
	* @returns {module:qm.StreamAggr} Self. The values in the stream aggregator are changed as defined in the inner onDelete function.
	*/
	//# exports.StreamAggr.prototype.onDelete = function (rec) { return Object.create(require('qminer').StreamAggr.prototype); };
	JsDeclareFunction(onDelete);
	
	//!- `objJSON = sa.saveJson(limit)` -- executes saveJson given an optional number parameter `limit`, whose meaning is specific to each type of stream aggregate
	/**
	* When executed it return a JSON object as defined by the user.
	* @param {number} [limit] - The meaning is specific to each type of stream aggregator.
	* @returns {Object} A JSON object as defined by the user.
	*/
	//# exports.StreamAggr.prototype.saveJson = function (limit) {};
	JsDeclareFunction(saveJson);

	//!- `fout = sa.save(fout)` -- executes save function given output stream `fout` as input. returns `fout`.
	/**
	* Saves the current state of the stream aggregator.
	* @param {module:fs.FOut} fout - The output stream.
	* @returns {module:fs.FOut} The output stream given as the parameter.
	*/
	//# exports.StreamAggr.prototype.save = function (fout) { return Object.create(require('qminer').fs.FOut.prototype); }
	JsDeclareFunction(save);
	
	//!- `sa = sa.load(fin)` -- executes load function given input stream `fin` as input. returns self.
	/**
	* Loads the stream aggregator, that has been previously saved.
	* @param {module:fs.FIn} fin - The input stream.
	* @returns {module:qm.StreamAggr} Self.
	*/
	//# exports.StreamAggr.prototype.load = function (fin) { return Object.create(require('qminer').StreamAggr.prototype); }
	JsDeclareFunction(load);
	// IInt
	//!- `num = sa.getInt()` -- returns a number if sa implements the interface IInt
	JsDeclareFunction(getInteger);

	// IFlt
	//!- `num = sa.getFlt()` -- returns a number if sa implements the interface IFlt
	/**
	* Returns the value of the specific stream aggregator. For return values see {@link module:qm~StreamAggregators}.
	* @returns {number} The value of the stream aggregator.
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
	*    winsize: 365 * 24 * 60 * 60 * 1000 
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
	*/
	//# exports.StreamAggr.prototype.getFloat = function () { return 0; };
	JsDeclareFunction(getFloat);

	// ITm
	//!- `num = sa.getTm()` -- returns a number if sa implements the interface ITm. The result is a windows timestamp (number of milliseconds since 1601)
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
	*    winsize: 31 * 60 * 60 * 1000
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
	*/
	//# exports.StreamAggr.prototype.getTimestamp = function () { return 0; };
	JsDeclareFunction(getTimestamp);
	
	// IFltVec
	//!- `num = sa.getFltLen()` -- returns a number (internal vector length) if sa implements the interface IFltVec.
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
	*    winsize: 24 * 60 * 60 * 1000
	* };
	* var icePrice = base.store('IceCreams').addStreamAggr(ts);
	* // add some ice creams in the store
	* base.store('IceCreams').push({ Type: 'Chocholate', Price: 5, TimeOfConsumption: '2015-07-21T09:00:00.0' });
	* base.store('IceCreams').push({ Type: 'Blue Sky', Price: 3, TimeOfConsumption: '2015-07-21T14:13:00.0' });
	* base.store('IceCreams').push({ Type: 'Stracciatella', Price: 5, TimeOfConsumption: '2015-07-21T21:05:00.0' });
	* // get the number of ice creams consumed by using getFloatLength method
	* var numberOfIceCreamsEaten = icePrice.getFloatLength(); // returns 3
	* 
	*/
	//# exports.StreamAggr.prototype.getFloatLength = function () { return 0; };
	JsDeclareFunction(getFloatLength);

	//!- `num = sa.getFltAt(idx)` -- returns a number (element at index) if sa implements the interface IFltVec.
	/**
	* Returns the value of the vector containing the values of the stream aggregator at a specific index.
	* @param {number} idx - The index.
	* @returns {number} The value of the float vector at position idx.
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
	*    winsize: 7 * 24 * 60 * 60 * 1000
	* };
	* var weekSales = base.store('MusicSale').addStreamAggr(ts);
	* // add some records in the store
	* base.store('MusicSale').push({ NumberOfAlbums: 10, Time: '2015-03-15T00:00:00.0' });
	* base.store('MusicSale').push({ NumberOfAlbums: 15, Time: '2015-03-18T00:00:00.0' });
	* base.store('MusicSale').push({ NumberOfAlbums: 30, Time: '2015-03-19T00:00:00.0' });
	* base.store('MusicSale').push({ NumberOfAlbums: 45, Time: '2015-03-20T00:00:00.0' });
	* // get the second value of the value vector 
	* var albums = weekSales.getFloatAt(1); // returns 15
	*/
	//# exports.StreamAggr.prototype.getFloatAt = function (idx) { return 0; };
	JsDeclareFunction(getFloatAt);
	
	//!- `vec = sa.getFltV()` -- returns a dense vector if sa implements the interface IFltVec.
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
	*    winsize: 7 * 24 * 60 * 60 * 1000
	* };
	* var weekPatients = base.store('Hospital').addStreamAggr(ts);
	* // add some records in the store
	* base.store('Hospital').push({ NumberOfPatients: 50, Date: '2015-05-20T00:00:00.0' });
	* base.store('Hospital').push({ NumberOfPatients: 56, Date: '2015-05-21T00:00:00.0' });
	* base.store('Hospital').push({ NumberOfPatients: 120, Date: '2015-05-22T00:00:00.0' });
	* base.store('Hospital').push({ NumberOfPatients: 40, Date: '2015-05-23T00:00:00.0' });
	* // get the values that are in the time series window buffer as a vector
	* var values = weekPatients.getFloatVector(); // returns the vector [50, 56, 120, 40]
	*/
	//# exports.StreamAggr.prototype.getFloatVector = function () { return Object.create(require('qminer').la.Vector.prototype); };
	JsDeclareFunction(getFloatVector);

	// ITmVec
	//!- `num = sa.getTmLen()` -- returns a number (timestamp vector length) if sa implements the interface ITmVec.
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
	*    winsize: 7 * 24 * 60 * 60 * 1000
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
	*/
	//# exports.StreamAggr.prototype.getTimestampLength = function () { return 0; };
	JsDeclareFunction(getTimestampLength);

	//!- `num = sa.getTmAt(idx)` -- returns a number (windows timestamp at index) if sa implements the interface ITmVec.
	/**
	* Gets the timestamp from the timestamp vector of the stream aggregator at the specific index.
	* @param {number} idx - The index.
	* @returns {number} The timestamp of the timestamp vector at position idx.
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
	*    winsize: 24 * 60 * 60 * 1000
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
	*/
	//# exports.StreamAggr.prototype.getTimestampAt = function (idx) { return 0; };
	JsDeclareFunction(getTimestampAt);

	//!- `vec = sa.getTmV()` -- returns a dense vector of windows timestamps if sa implements the interface ITmVec.
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
	*    winsize: 10 * 1000
	* };
	* var signalBeep = base.store('Signals').addStreamAggr(ts);
	* // add some records to the store
	* base.store('Signals').push({ BeepLoudness: 10, Time: '2015-07-21T12:30:30.0' });
	* base.store('Signals').push({ BeepLoudness: 25, Time: '2015-07-21T12:30:31.0' });
	* base.store('Signals').push({ BeepLoudness: 20, Time: '2015-07-21T12:30:32.0' });
	* // get the timestamp vector of signalBeep
	* var vec = signalBeep.getTimestampVector(); // returns vector [13081955430000, 13081955431000, 13081955432000]
	*/
	//# exports.StreamAggr.prototype.getTimestampVector = function () { return Object.create(require('qminer').la.Vector.prototype); };
	JsDeclareFunction(getTimestampVector);
	
	// IFltTmIO
	//!- `num = sa.getInFlt()` -- returns a number (input value arriving in the buffer) if sa implements the interface IFltTmIO.
	/**
	* Gets the value of the newest record added to the stream aggregator.
	* @returns {number} The value of the newest record in the buffer.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a simple base containing one store
	* var base = new qm.Base({
	*    mode: 'createClean',
	*    schema: [{
	*        name: 'Marathon',
	*        fields: [
	*            { name: 'Runner', type: 'string' },
	*            { name: 'Speed', type: 'float' },
	*            { name: 'Time', type: 'datetime' }
	*        ]
	*    }]
	* });
	* // create a time series stream aggregator that gets the values from the 'Speed' field
	* // and the timestamp from the 'Time' field. The window size should be 10 minutes.
	* var ts = {
	*    name: 'Sensor',
	*    type: 'timeSeriesWinBuf',
	*    store: 'Marathon',
	*    timestamp: 'Time',
	*    value: 'Speed',
	*    winsize: 10 * 60 * 1000
	* };
	* var sensor = base.store('Marathon').addStreamAggr(ts);
	* // add some records to the store
	* base.store('Marathon').push({ Runner: 'Marko Primozic', Speed: 13.4, Time: '2015-07-21T20:23:13.0' });
	* base.store('Marathon').push({ Runner: 'Leonard Cohen', Speed: 14.1, Time: '2015-07-21T20:24:01.0' });
	* base.store('Marathon').push({ Runner: 'Coco Chanelle', Speed: 13.7, Time: '2015-07-21T20:24:27.0' });
	* // get the last value that got in the buffer
	* var last = sensor.getInFloat(); // returns 13.7
	*/
	//# exports.StreamAggr.prototype.getInFloat = function () { return 0; };
	JsDeclareFunction(getInFloat);

	//!- `num = sa.getInTm()` -- returns a number (windows timestamp arriving in the buffer) if sa implements the interface IFltTmIO.
	/**
	* Gets the timestamp of the newest record added to the stream aggregator.
	* @returns {number} The timestamp given as the number of miliseconds since 01.01.1601, time: 00:00:00.0.
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
	*    winsize: 5 * 60 * 1000
	* };
	* var sensor = base.store('F1').addStreamAggr(ts);
	* // add some records to the store
	* base.store('F1').push({ Driver: 'Sebastian Vettel', Speed: 203.4, Time: '2015-07-19T09:32:01.0' });
	* base.store('F1').push({ Driver: 'Thomas "Tommy" Angelo', Speed: 152.8, Time: '2015-07-19T09:35:23.0' });
	* base.store('F1').push({ Driver: 'Mark Ham', Speed: 189.5, Time: '2015-07-19T09:38:43.0' });
	* // get the last timestamp that was added in the window buffer
	* var time = sensor.getInTimestamp(); // returns 13081772323000
	*/
	//# exports.StreamAggr.prototype.getInTimestamp = function () { return 0; };
	JsDeclareFunction(getInTimestamp);
	
	//!- `vec = sa.getOutFltV()` -- returns a dense vector (values leaving the buffer) if sa implements the interface IFltTmIO.
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
	*    winsize: 5 * 60 * 1000
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
	*/
	//# exports.StreamAggr.prototype.getOutFloatVector = function () { return Object.create(require('qminer').la.Vector.prototype); };
	JsDeclareFunction(getOutFloatVector);

	//!- `vec = sa.getOutTmV()` -- returns a dense vector (windows timestamps leaving the bugger) if sa implements the interface IFltTmIO.
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
	*/
	//# exports.StreamAggr.prototype.getOutTimestampVector = function () { return Object.create(require('qminer').la.Vector.prototype); };
	JsDeclareFunction(getOutTimestampVector);

	//!- `num = sa.getN()` -- returns a number of records in the input buffer if sa implements the interface IFltTmIO.
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
	*    winsize: 7 * 24 * 60 * 60 * 1000
	* };
	* var weekSales = base.store('MusicSale').addStreamAggr(ts);
	* // add some records in the store
	* base.store('MusicSale').push({ NumberOfAlbums: 10, Time: '2015-03-15T00:00:00.0' });
	* base.store('MusicSale').push({ NumberOfAlbums: 15, Time: '2015-03-18T00:00:00.0' });
	* base.store('MusicSale').push({ NumberOfAlbums: 30, Time: '2015-03-19T00:00:00.0' });
	* base.store('MusicSale').push({ NumberOfAlbums: 45, Time: '2015-03-20T00:00:00.0' });
	* // get the number of records in the window buffer
	* var num = weekSales.getNumberOfRecords(); // returns 4
	*/
	//# exports.StreamAggr.prototype.getNumberOfRecords = function () { return 0; };
	JsDeclareFunction(getNumberOfRecords);

	//!- `str = sa.name` -- returns the name (unique) of the stream aggregate
	/**
	* Returns the name of the stream aggregate.
	*/
	//# exports.StreamAggr.prototype.name = "";
	JsDeclareProperty(name);

	//!- `objJSON = sa.val` -- same as sa.saveJson(-1)
	/**
	* Returns the JSON object of the stream aggregate. Same as the method saveJson.
	*/
	//# exports.StreamAggr.prototype.val = undefined;
	JsDeclareProperty(val);
};

///////////////////////////////
// JavaScript Stream Aggregator
class TNodeJsStreamAggr :
	public TQm::TStreamAggr,
	public TQm::TStreamAggrOut::IInt,
	//public TQm::TStreamAggrOut::IFlt,	
	//public TQm::TStreamAggrOut::ITm,
	public TQm::TStreamAggrOut::IFltTmIO,
	public TQm::TStreamAggrOut::IFltVec,
	public TQm::TStreamAggrOut::ITmVec,
	public TQm::TStreamAggrOut::INmFlt,
	public TQm::TStreamAggrOut::INmInt,
	// combinations
	public TQm::TStreamAggrOut::IFltTm
	//public TQm::TStreamAggrOut::IFltVecTm
{
private:	
	// callbacks
	v8::Persistent<v8::Function> OnAddFun;
	v8::Persistent<v8::Function> OnUpdateFun;
	v8::Persistent<v8::Function> OnDeleteFun;
	v8::Persistent<v8::Function> SaveJsonFun;

	v8::Persistent<v8::Function> GetIntFun;
	// IFlt 
	v8::Persistent<v8::Function> GetFltFun;
	// ITm 
	v8::Persistent<v8::Function> GetTmMSecsFun;
	// IFltTmIO 
	v8::Persistent<v8::Function> GetInFltFun;
	v8::Persistent<v8::Function> GetInTmMSecsFun;
	v8::Persistent<v8::Function> GetOutFltVFun;
	v8::Persistent<v8::Function> GetOutTmMSecsVFun;
	v8::Persistent<v8::Function> GetNFun;
	v8::Persistent<v8::Function> GetOldestFltFun;
	v8::Persistent<v8::Function> GetOldestTmMSecsFun;

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
	v8::Persistent<v8::Function> GetNmFltVFun;
	// INmInt
	v8::Persistent<v8::Function> IsNmFun;
	v8::Persistent<v8::Function> GetNmIntFun;
	v8::Persistent<v8::Function> GetNmIntVFun;
	// Serialization
	v8::Persistent<v8::Function> SaveFun;
	v8::Persistent<v8::Function> LoadFun;

public:
	TNodeJsStreamAggr(TWPt<TQm::TBase> _Base, const TStr& _AggrNm, v8::Handle<v8::Object> TriggerVal);
	static TQm::PStreamAggr New(TWPt<TQm::TBase> _Base, const TStr& _AggrNm, v8::Handle<v8::Object> TriggerVal) {
		return new TNodeJsStreamAggr(_Base, _AggrNm, TriggerVal);
	}

	~TNodeJsStreamAggr();

	void OnAddRec(const TQm::TRec& Rec);
	void OnUpdateRec(const TQm::TRec& Rec);
	void OnDeleteRec(const TQm::TRec& Rec);
	PJsonVal SaveJson(const int& Limit) const;

	// stream aggregator type name 
	static TStr GetType() { return "javaScript"; }
	TStr Type() const { return GetType(); }
	void _Save(TSOut& SOut) const;
	void _Load(TSIn& SIn);

	// IInt
	int GetInt() const;
	// IFlt 
	double GetFlt() const;
	// ITm 
	uint64 GetTmMSecs() const;
	// IFltTmIO 
	double GetInFlt() const;
	uint64 GetInTmMSecs() const;
	void GetOutFltV(TFltV& ValV) const;
	void GetOutTmMSecsV(TUInt64V& MSecsV) const;
	int GetN() const;
	double GetOldestFlt() const;
	uint64 GetOldestTmMSecs() const;

	// IFltVec
	int GetFltLen() const;
	double GetFlt(const TInt& ElN) const; // GetFltAtFun
	void GetFltV(TFltV& ValV) const;
	// ITmVec
	int GetTmLen() const;
	uint64 GetTm(const TInt& ElN) const; // GetTmAtFun
	void GetTmV(TUInt64V& TmMSecsV) const;
	// INmFlt 
	bool IsNmFlt(const TStr& Nm) const;
	double GetNmFlt(const TStr& Nm) const;
	void GetNmFltV(TStrFltPrV& NmFltV) const;
	// INmInt
	bool IsNm(const TStr& Nm) const;
	double GetNmInt(const TStr& Nm) const;
	void GetNmIntV(TStrIntPrV& NmIntV) const;
};

#endif

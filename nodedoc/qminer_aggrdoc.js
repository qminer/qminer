/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
/**
* Qminer module.
* @module qm
* @example
* // import module
* var qm = require('qminer');
*/
/**
* Stream Aggregate
* @classdesc Represents the stream aggregate. The class can construct these {@link module:qm~StreamAggregators}.
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
*    schema: [
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
* // create a new stream aggregator for "People" store: get the length of the record name (with the function object)
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
* // create a new stream aggregator for "Laser" store: timeseries window buffer (with the JSON object)
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
 exports.StreamAggr = function (base, json, storeName) {};
/**
* @typedef {module:qm.StreamAggr} StreamAggregators
* Stream aggregator types.
* @property {module:qm~StreamAggr_TimeSeries} timeSeries - The time series type.
* @property {module:qm~StreamAggr_Count} count - The count type.
* @property {module:qm~StreamAggr_Sum} sum - The sum type.
* @property {module:qm~StreamAggr_Min} min - The minimal type.
* @property {module:qm~StreamAggr_Max} max - The maximal type.
* @property {module:qm~StreamAggr_TimeSeriesTick} tick - The time series tick type.
* @property {module:qm~StreamAggr_MovingAverage} ma - The moving average type.
* @property {module:qm~StreamAggr_EMA} ema - The exponental moving average type.
* @property {module:qm~StreamAggr_MovingVariance} var - The moving variance type.
* @property {module:qm~StreamAggr_MovingCovariance} cov - The moving covariance type.
* @property {module:qm~StreamAggr_MovingCorrelation} cor - The moving correlation type.
* @property {module:qm~StreamAggr_Resampler} res - The resampler type.
* @property {module:qm~StreamAggr_Merger} mer - The merger type.
*/
/**
* @typedef {module:qm.StreamAggr} StreamAggr_TimeSeries
* This stream aggregator represents the time series window buffer. It implements all functions of the {@link module:qm.StreamAggr}.
* @property {string} name - The given name of the stream aggregator.
* @property {string} type - The type of the stream aggregator. It must be equal to 'timeSeriesWinBuf'.
* @property {string} store - The name of the store from which to takes the data.
* @property {string} timestamp - The field of the store, where it takes the time/date.
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
* // create a new time series stream aggregator for the Heat store, that takes the values from the Celcius field
* // and the timestamp from the Time field. The size of the window is 2 seconds (2000ms)
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
* @typedef {module:qm.StreamAggr} StreamAggr_Count
* This stream aggregator represents the count moving window buffer. It implements the following methods:
* <br>{@link module:qm.StreamAggr#getFloat} returns the number of records in the it's buffer window. 
* <br>{@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in the it's buffer window.
* @property {string} name - The given name of the stream aggregator.
* @property {string} type - The type of the stream aggregator. It must be equal to 'winBufCount'.
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
*        name: "Heat",
*        fields: [
*            { name: "Celcius", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
* // create a new time series stream aggregator for the Heat store, that takes the values from the Celcius field
* // and the timestamp from the Time field. The size of the window is 2 seconds (2000ms)
* var timeser = {
*    name: 'TimeSeriesAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celcius',
*    winsize: 2 * 1000
* };
* var timeSeries = base.store("Heat").addStreamAggr(timeser);
* // add a count aggregator, that is connected with the TimeSeriesAggr
* var co = {
*    name: 'CountAggr',
*    type: 'winBufCount',
*    store: 'Heat',
*    inAggr: 'TimeSeriesAggr'
* };
* var count = base.store("Heat").addStreamAggr(co);
*/
/**
* @typedef {module:qm.StreamAggr} StreamAggr_Sum
* This stream aggregator represents the sum moving window buffer. It implements the following methods:
* <br>{@link module:qm.StreamAggr#getFloat} returns the sum of the values of the records in the it's buffer window.
* <br>{@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in the it's buffer window.
* @property {string} name - The given name of the stream aggregator.
* @property {string} type - The type of the stream aggregator. It must be equal to 'winBufSum'.
* @property {string} store - The name of the store form which it takes the data.
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
* // create a new time series stream aggregator for the Heat store, that takes the values from the Celcius field
* // and the timestamp from the Time field. The size of the window is 2 seconds (2000ms)
* var timeser = {
*    name: 'TimeSeriesAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celcius',
*    winsize: 2 * 1000
* };
* var timeSeries = base.store("Heat").addStreamAggr(timeser);
* // add a sum aggregator, that is connected with the TimeSeriesAggr
* var sum = {
*    name: 'SumAggr',
*    type: 'winBufSum',
*    store: 'Heat',
*    inAggr: 'TimeSeriesAggr'
* };
* var sumAggr = base.store("Heat").addStreamAggr(sum);
*/
/**
* @typedef {module:qm.StreamAggr} StreamAggr_Min
* This stream aggregator represents the minimal moving window buffer. It implements the following methods:
* <br>{@link module:qm.StreamAggr#getFloat} returns the minimal value of the records in it's buffer window.
* <br>{@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in it's buffer window.
* @property {string} name - The given name of the stream aggregator.
* @property {string} type - The type of the stream aggregator. It must be equal to 'winBufMin'.
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
* // create a new time series stream aggregator for the Heat store, that takes the values from the Celcius field
* // and the timestamp from the Time field. The size of the window is 2 seconds (2000ms)
* var timeser = {
*    name: 'TimeSeriesAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celcius',
*    winsize: 2 * 1000
* };
* var timeSeries = base.store("Heat").addStreamAggr(timeser);
* // add a min aggregator, that is connected with the TimeSeriesAggr
* var min = {
*    name: 'MinAggr',
*    type: 'winBufMin',
*    store: 'Heat',
*    inAggr: 'TimeSeriesAggr'
* };
* var minimal = base.store("Heat").addStreamAggr(min);
*/
/**
* @typedef {module:qm.StreamAggr} StreamAggr_Max
* This stream aggregator represents the maximal moving window buffer. It implements the following methods:
* <br>{@link module:qm.StreamAggr#getFloat} returns the maximal value of the records in the it's buffer window.
* <br>{@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in it's buffer window.
* @property {string} name - The given name of the stream aggregator.
* @property {string} type - The type for the stream aggregator. It must be equal to 'winBufMax'.
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
* // create a new time series stream aggregator for the Heat store, that takes the values from the Celcius field
* // and the timestamp from the Time field. The size of the window is 2 seconds (2000ms)
* var timeser = {
*    name: 'TimeSeriesAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celcius',
*    winsize: 2 * 1000
* };
* var timeSeries = base.store("Heat").addStreamAggr(timeser);
* // add a max aggregator, that is connected with the TimeSeriesAggr
* var max = {
*    name: 'MaxAggr',
*    type: 'winBufMax',
*    store: 'Heat',
*    inAggr: 'TimeSeriesAggr'
* };
* var maximal = base.store("Heat").addStreamAggr(max);
*/
/**
* @typedef {module:qm.StreamAggr} StreamAggr_TimeSeriesTick
* This stream aggregator represents the time series tick window buffer. It implements the following methods:
* <br>{@link module:qm.StreamAggr#getFloat} returns the last value added in the it's window buffer.
* <br>{@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in it's buffer window.
* @property {string} name - The given name for the stream aggregator.
* @property {string} type - The type of the stream aggregator. It must be equal to 'timeSeriesTick'.
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
*        name: "Heat",
*        fields: [
*            { name: "Celcius", type: "float" },
*            { name: "Time", type: "datetime" }
*        ]
*    }]
* });
* // create a new time series stream aggregator for the Heat store, that takes the values from the Celcius field
* // and the timestamp from the Time field. The size of the window is 2 seconds (2000ms)
* var tick = {
*    name: 'TimeSeriesTickAggr',
*    type: 'timeSeriesTick',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celcius',
* };
* var timeSeriesTick = base.store("Heat").addStreamAggr(tick);
*/
/**
* @typedef {module:qmStreamAggr} StreamAggr_MovingAverage
* This stream aggregator represents the moving average window buffer. It implements the following methods:
* <br>{@link module:qm.StreamAggr#getFloat} returns the average of the values in it's buffer window.
* <br>{@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in it's buffer window.
* @property {string} name - The given name of the stream aggregator.
* @property {string} type - The type of the stream aggregator. It must be equal to 'ma'.
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
* // create a new time series stream aggregator for the Heat store, that takes the values from the Celcius field
* // and the timestamp from the Time field. The size of the window is 2 seconds (2000ms)
* var timeser = {
*    name: 'TimeSeriesAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celcius',
*    winsize: 2 * 1000
* };
* var timeSeries = base.store("Heat").addStreamAggr(timeser);
* // add a moving average aggregator, that is connected with the TimeSeriesAggr
* var ma = {
*    name: 'movingAverageAggr',
*    type: 'ma',
*    store: 'Heat',
*    inAggr: 'TimeSeriesAggr'
* };
* var movingAverage = base.store("Heat").addStreamAggr(ma);
*/
/**
* @typedef {module:qmStreamAggr} StreamAggr_EMA
* This stream aggregator represents the exponential moving average window buffer. It implements the following methods:
* <br>{@link module:qm.StreamAggr#getFloat} returns the exponentional average of the values in it's buffer window.
* <br>{@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in it's buffer window.
* @property {string} name - The given name for the stream aggregator.
* @property {string} type - The type of the stream aggregator. It must be equal to 'ema'.
* @property {string} store - The name of the store from which it takes the data.
* @property {string} inAggr - The name of the stream aggregator to which it connects and gets data.
* @property {string} emaType - The type of interpolation. The choices are: 'previous', 'linear' and 'next'.
* @property {number} interval - The time interval defining the decay. It must be greater than initWindow.
* @property {number} initWindow -
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
* // create a new time series tick stream aggregator for the Heat store, that takes the values from the Celcius field
* // and the timestamp from the Time field. The size of the window is 5 seconds (5000ms)
* var timeser = {
*    name: 'TimeSeriesAggr',
*    type: 'timeSeriesTick',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celcius',
*    winsize: 5 * 1000
* };
* var timeSeries = base.store("Heat").addStreamAggr(timeser);
* // add an exponentional moving average aggregator, that is connected with the TimeSeriesAggr
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
* @typedef {module:qm.StreamAggr} StreamAggr_MovingVariance
* This stream aggregator represents the moving variance window buffer. It implements the following methods:
* <br>{@link module:qm.StreamAggr#getFloat} returns the variance of the values in it's buffer window.
* <br>{@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in it's buffer window.
* @property {string} name - The given name for the stream aggregator.
* @property {string} type - The type of the stream aggregator. It must be equal to 'variance'.
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
* // create a new time series stream aggregator for the Heat store, that takes the values from the Celcius field
* // and the timestamp from the Time field. The size of the window is 2 seconds (2000ms)
* var timeser = {
*    name: 'TimeSeriesAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celcius',
*    winsize: 2 * 1000
* };
* var timeSeries = base.store("Heat").addStreamAggr(timeser);
* // add a variance aggregator, that is connected with the TimeSeriesAggr
* var variance = {
*    name: 'varAggr',
*    type: 'variance',
*    store: 'Heat',
*    inAggr: 'TimeSeriesAggr'
* };
* var varianceAggr = base.store("Heat").addStreamAggr(variance);
*/
/**
* @typedef {module:qm.StreamAggr} StreamAggr_MovingCovariance
* This stream aggregator represents the moving covariance window buffer. It implements the following methods:
* <br>{@link module:qm.StreamAggr#getFloat} returns the covariance of the values in it's buffer window.
* <br>{@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in it's buffer window.
* @property {string} name - The given name for the stream aggregator.
* @property {string} type - The type of the stream aggregator. It must be equal to 'covariance'.
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
* // create a new time series stream aggregator for the Heat store, that takes the values from the Celcius field
* // and the timestamp from the Time field. The size of the window is 2 seconds (2000ms)
* var celcius = {
*    name: 'CelciusAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celcius',
*    winsize: 2 * 1000
* }; base.store("Heat").addStreamAggr(celcius);
* // create a new time series stream aggregator for the Heat store, that takes the values from the WaterConsumption field
* // and the timestamp from the Time field. The size of the window is 2 seconds (2000ms)
* var water = {
*    name: 'WaterAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'WaterConsumption',
*    winsize: 2 * 1000
* }; base.store("Heat").addStreamAggr(water);
* // add a covariance aggregator, that is connected with the celciusAggr and waterAggr
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
* @typedef {module:qm.StreamAggr} StreamAggr_MovingCorrelation
* This stream aggregator represents the moving covariance window buffer. It implements the following methods:
* <br>{@link module:qm.StreamAggr#getFloat} returns the correlation of the values in it's buffer window.
* <br>{@link module:qm.StreamAggr#getTimestamp} returns the timestamp of the newest record in it's buffer window.
* @property {string} name - The given name for the stream aggregator.
* @property {string} type - The type of the stream aggregator. It must be equal to 'correlation'.
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
* // create a new time series stream aggregator for the Heat store, that takes the values from the Celcius field
* // and the timestamp from the Time field. The size of the window is 2 seconds (2000ms)
* var celcius = {
*    name: 'CelciusAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'Celcius',
*    winsize: 2 * 1000
* }; base.store("Heat").addStreamAggr(celcius);
* // create a new time series stream aggregator for the Heat store, that takes the values from the WaterConsumption field
* // and the timestamp from the Time field. The size of the window is 2 seconds (2000ms)
* var water = {
*    name: 'WaterAggr',
*    type: 'timeSeriesWinBuf',
*    store: 'Heat',
*    timestamp: 'Time',
*    value: 'WaterConsumption',
*    winsize: 2 * 1000
* }; base.store("Heat").addStreamAggr(water);
* // add a covariance aggregator, that is connected with the celciusAggr and waterAggr
* var covariance = {
*    name: 'covarianceAggr',
*    type: 'covariance',
*    store: 'Heat',
*    inAggrX: 'CelciusAggr',
*    inAggrY: 'WaterAggr'
* }; base.store("Heat").addStreamAggr(covariance);
* // add the two variance aggregators, that take from the Celcius and WaterConsumption, respectively.
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
* // add a correlation aggregator, that is connected to covarianceAggr, celciusVarAggr and waterValAggr
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
* @typedef {module:qm.StreamAggr} StreamAggr_Resampler
* This stream aggregator represents the resampler window buffer. No methods are implemented for this aggregator.
* @property {string} name - The given name for the stream aggregator.
* @property {string} type - The type of the stream aggregator. It must be equal to 'resampler'.
* @property {string} store - The name of the store from which it takes the data.
* @property {string} outStore - The store in which the samples are stored.
* @property {string} timestamp - The store field from which it takes the timestamps.
* @property {Object} fields - The json, which contains:
* <br> name (string) - the store field from which it takes the values.
* <br> interpolator (string) - the type of the interpolation. The options are: 'previous', 'next' and 'linear'.
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
* // create a new resampler stream aggregator for the Heat store, that takes the values from the Celcius field
* // and the timestamp from the Time field. The interpolated values are stored in the 'interpolatedValues' store 
* // the interpolation should be linear
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
* @typedef {module:qm.StreamAggr} StreamAggr_Merger
* This stream aggregator represents the merger aggregator. It merges records from two or more stores into a new store
* depending on the timestamp. No methods are implemented for this aggregator.
* @property {string} name - The given name for the stream aggregator.
* @property {string} type - The type of the stream aggregator. It must be equal to 'stmerger'.
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
* // create a new merger stream aggregator that mergers the records of the "Cars" and "Temperature" stores.
* // The records are interpolated linearly and stored in the "Merged" store.
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
/**
	* Executes the function when a new record is put in store.
	* @param {module:qm.Record} rec - The record given to the stream aggregator.
	* @returns {module:qm.StreamAggr} Self. Values in the stream aggregator are changed as defined in the inner onUpdate function.
	*/
 exports.StreamAggr.prototype.onAdd = function (rec) { return Object.create(require('qminer').StreamAggr.prototype); };
/**
	* Executes the function when a record in the store is updated.
	* @param {module:qmRecord} rec - The updated record given to the stream aggregator.
	* @returns {module:qm.StreamAggr} Self. Values in the stream aggregator are changed as defined in the inner onUpdate function.
	*/
 exports.StreamAggr.prototype.onUpdate = function (rec) { return Object.create(require('qminer').StreamAggr.prototype); };
/**
	* Executes the function when a record in the store is deleted.
	* @param {module:qm.Record} rec - The deleted record given to the stream aggregator.
	* @returns {module:qm.StreamAggr} Self. The values in the stream aggregator are changed as defined in the inner onDelete function.
	*/
 exports.StreamAggr.prototype.onDelete = function (rec) { return Object.create(require('qminer').StreamAggr.prototype); };
/**
	* When executed it return a JSON object as defined by the user.
	* @param {number} [limit] - The meaning is specific to each type of stream aggregator. //TODO
	* @returns {Object} A JSON object as defined by the user.
	*/
 exports.StreamAggr.prototype.saveJson = function (limit) {};
/**
	* Returns the float value of the newest record in buffer.
	* @returns {number} The value of the newest record.
	*/
 exports.StreamAggr.prototype.getFloat = function () { return 0; };
/**
	* Returns the timestamp value of the newest record in buffer.
	* @returns {number} The timestamp of the newest record. It represents the number of miliseconds between the records time and 1601-01-01T00:00:00.0.
	*/
 exports.StreamAggr.prototype.getTimestamp = function () { return 0; };
/**
	* Gets the length of the value vector.
	* @returns {number} The length of the value vector.
	*/
 exports.StreamAggr.prototype.getFloatLength = function () { return 0; };
/**
	* Returns the value of the float vector at a specific index.
	* @param {number} idx - The index.
	* @returns {number} The value of the float vector at position idx.
	*/
 exports.StreamAggr.prototype.getFloatAt = function (idx) { return 0; };
/**
	* Gets the whole value vector of the buffer.
	* @returns {module:la.Vector} The vector containing the values of the buffer.
	*/
 exports.StreamAggr.prototype.getFloatVector = function () { return Object.create(require('qminer').la.Vector.prototype); };
/**
	* Gets the length of the timestamp vector.
	* @returns {number} The length of the timestamp vector.
	*/
 exports.StreamAggr.prototype.getTimestampLength = function () { return 0; };
/**
	* Gets the timestamp on the specific location in the buffer.
	* @param {number} idx - The index.
	* @returns {number} The timestamp located on the idx position in the buffer.
	*/
 exports.StreamAggr.prototype.getTimestampAt = function (idx) { return 0; };
/**
	* Gets the dense vector containing the windows timestamps.
	* @returns {module:la.Vector} The vector containing the window timestamps.
	*/
 exports.StreamAggr.prototype.getTimestampVector = function () { return Object.create(require('qminer').la.Vector.prototype); };
/**
	* Gets the value of the newest record added to the buffer.
	* @returns {number} The value of the newest record in the buffer.
	*/
 exports.StreamAggr.prototype.getInFloat = function () { return 0; };
/**
	* Gets the timestamp of the newest record added to the buffer.
	* @returns {number} The timestamp given as the number of miliseconds since 01.01.1601, time: 00:00:00.0.
	*/
 exports.StreamAggr.prototype.getInTimestamp = function () { return 0; };
/**
	* Gets a vector containing the values that are leaving the buffer.
	* @returns {module:la.Vector} The vector containing the values that are leaving the buffer.
	*/
 exports.StreamAggr.prototype.getOutFloatVector = function () { return Object.create(require('qminer').la.Vector.prototype); };
/**
	* Gets a vector containing the timestamps that are leaving the buffer.
	* @returns {module:la.Vector} The vector containing the leaving timestamps.
	*/
 exports.StreamAggr.prototype.getOutTimestampVector = function () { return Object.create(require('qminer').la.Vector.prototype); };
/**
	* Gets the number of records in the buffer.
	* @returns {number} The number of records in the buffer.
	*/
 exports.StreamAggr.prototype.getNumberOfRecords = function () { return 0; };
/**
	* Returns the name of the stream aggregate.
	*/
 exports.StreamAggr.prototype.name = undefined;
/**
	* Returns the JSON object of the stream aggregate. Same as the method saveJson.
	*/
 exports.StreamAggr.prototype.val = undefined;

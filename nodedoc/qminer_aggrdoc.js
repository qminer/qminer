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
* @classdesc Represents the stream aggregate.
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
* @property {module:qm~StreamAggr_MovingAverage} ma - The moving average type.
* @property {module:qm~StreamAggr_EMA} ema - The exponental moving average type.
* @property {module:qm~StreamAggr_MovingVariance} var - The moving variance type.
* @property {module:qm~StreamAggr_MovingCovariance} cov - The moving covariance type.
* @property {module:qm~StreamAggr_MovingCorrelation} cor - The moving correlation type.
* @property {module:qm~StreamAggr_Resampler} res - The resampler type.
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

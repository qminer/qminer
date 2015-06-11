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
*            { name: "Gendre", type: "string" }
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
* // create a new stream aggregator for "People" store: get gendre (with the JSON object)
* var aggr2 = new qm.StreamAggr(base, { type: , name: 'gendreSeperator', outStore: 'People', createStore: false}
*/
 exports.StreamAggr = function (base, json, storeName) {};
/**
	* Executes the function when a new record is put in store.
	* @param {module:qm.Record} rec - The record given to the stream aggregator.
	* @returns {module:qm.StreamAggr} Self. Values in the stream aggregator are changed as defined in the inner onUpdate function.
	*/
 exports.StreamAggr.prototype.onAdd = function (rec) {};
/**
	* Executes the function when a record is the store is updated.
	* @param {module:qmRecord} rec - The updated record given to the stream aggregator.
	* @return {module:qm.StreamAggr} Self. Values in the stream aggregator are changed as defined in the inner onUpdate function.
	*/
 exports.StreamAggr.prototype.onUpdate = function (rec) {};
/**
	* When executed it return a JSON object as defined by the user.
	* @param {number} [limit] - The meaning is specific to each type of stream aggregator. //TODO
	* @returns {Object} A JSON object as defined by the user.
	*/
 exports.StreamAggr.prototype.saveJson = function (limit) {};
/**
	* Gets the length of the value vector.
	* @returns {number} The length of the value vector.
	*/
 exports.StreamAggr.prototype.getFloatLength = function () {};
/**
	* Returns the value of the float vector at a specific index.
	* @param {number} idx - The index.
	* @returns {number} The value of the float vector at position idx.
	*/
 exports.StreamAggr.prototype.getFloatAt = function (idx) {};
/**
	* Gets the whole value vector of the buffer.
	* @returns {module:la.Vector} The vector containing the values of the buffer.
	*/
 exports.StreamAggr.prototype.getFloatVector = function () {};
/**
	* Gets the length of the timestamp vector.
	* @returns {number} The length of the timestamp vector.
	*/
 exports.StreamAggr.prototype.getTimestampLength = function () {};
/**
	* Gets the timestamp on the specific location in the buffer.
	* @param {number} idx - The index.
	* @returns {number} The timestamp located on the idx position in the buffer.
	*/
 exports.StreamAggr.prototype.getTimestampAt = function (idx) {};
/**
	* Gets the dense vector containing the windows timestamps.
	* @returns {module:la.Vector} The vector containing the window timestamps.
	*/
 exports.StreamAggr.prototype.getTimestampVector = function () {};
/**
	* Gets the value of the newest record added to the buffer.
	* @returns {number} The value of the newest record in the buffer.
	*/
 exports.StreamAggr.prototype.getInFloat = function () {};
/**
	* Gets the timestamp of the newest record added to the buffer.
	* @returns {number} The timestamp given as the number of miliseconds since 01.01.1601, time: 00:00:00.0.
	*/
 exports.StreamAggr.prototype.getInTimestamp = function () {};
/**
	* Gets a vector containing the values that are leaving the buffer.
	* @returns {module:la.Vector} The vector containing the values that are leaving the buffer.
	*/
 exports.StreamAggr.prototype.getOutFloatVector = function () {};
/**
	* Gets a vector containing the timestamps that are leaving the buffer.
	* @returns {module:la.Vector} The vector containing the leaving timestamps.
	*/
 exports.StreamAggr.prototype.getOutTimestampVector = function () {};

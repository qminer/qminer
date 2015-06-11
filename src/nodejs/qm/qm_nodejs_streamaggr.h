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

//! 
//! ### NodeJs Stream Aggregate
//! 
//! Stream aggregates are objects used for processing data streams - their main functionality includes four functions: onAdd, onUpdate, onDelete process a record, and saveJson which returns a JSON object that describes the aggregate's state.

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
//# exports.StreamAggr = function (base, json, storeName) {};
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
	//!- `sa = new qm.sa(base, paramJSON)` -- create a new [Stream Aggregate](Stream-Aggregates) object `sa`. The constructor parameters are stored in `paramJSON` object. `paramJSON` must contain field `type` which defines the type of the aggregate.
	//!- `sa = new qm.sa(base, paramJSON, storeName)` -- create a new [Stream Aggregate](Stream-Aggregates) object `sa`. The constructor parameters are stored in `paramJSON` object. `paramJSON` must contain field `type` which defines the type of the aggregate. Second parameter `storeName` is used to register the stream aggregate for events on the appropriate store.
	//!- `sa = new qm.sa(base, paramJSON, storeNameArr)` -- create a new [Stream Aggregate](Stream-Aggregates) object `sa`. The constructor parameters are stored in `paramJSON` object. `paramJSON` must contain field `type` which defines the type of the aggregate. Second parameter `storeNameArr` is an array of store names, where the stream aggregate will be registered.
	//!- `sa = new qm.sa(base, funObj)` -- create a new [Stream Aggregate](Stream-Aggregates). The function object `funObj` defines the aggregate name and four callbacks: onAdd (takes record as input), onUpdate (takes record as input), onDelete (takes record as input) and saveJson (takes one numeric parameter - limit) callbacks. An example: `funObj = new function () {this.name = 'aggr1'; this.onAdd = function (rec) { }; this.onUpdate = function (rec) { }; this.onDelete = function (rec) { };  this.saveJson = function (limit) { return {}; } }`.
	//!- `sa = new qm.sa(base, funObj, storeName)` -- create a new [Stream Aggregate](Stream-Aggregates). The function object `funObj` defines the aggregate name and four callbacks: onAdd (takes record as input), onUpdate (takes record as input), onDelete (takes record as input) and saveJson (takes one numeric parameter - limit) callbacks. An example: `funObj = new function () {this.name = 'aggr1'; this.onAdd = function (rec) { }; this.onUpdate = function (rec) { }; this.onDelete = function (rec) { };  this.saveJson = function (limit) { return {}; } }`.  Second parameter `storeName` is used to register the stream aggregate for events on the appropriate store.
	//!- `sa = new qm.sa(base, funObj, storeNameArr)` -- create a new [Stream Aggregate](Stream-Aggregates). The function object `funObj` defines the aggregate name and four callbacks: onAdd (takes record as input), onUpdate (takes record as input), onDelete (takes record as input) and saveJson (takes one numeric parameter - limit) callbacks. An example: `funObj = new function () {this.name = 'aggr1'; this.onAdd = function (rec) { }; this.onUpdate = function (rec) { }; this.onDelete = function (rec) { };  this.saveJson = function (limit) { return {}; } }`.  Second parameter `storeNameArr` is an array of store names, where the stream aggregate will be registered.
	//!- `sa = new qm.sa(base, ftrExtObj)` -- create a new [Stream Aggregate](Stream-Aggregates). The `ftrExtObj = {type : 'ftrext', name : 'aggr1', featureSpace: fsp }` object has three parameters: `type='ftrext'`,`name` (string) and feature space `featureSpace` whose value is a feature space object.
	//!- `sa = new qm.sa(base, ftrExtObj, storeName)` -- create a new [Stream Aggregate](Stream-Aggregates). The `ftrExtObj = {type : 'ftrext', name : 'aggr1', featureSpace: fsp }` object has three parameters: `type='ftrext'`,`name` (string) and feature space `featureSpace` whose value is a feature space object.  Second parameter `storeName` is used to register the stream aggregate for events on the appropriate store.
	//!- `sa = new qm.sa(base, ftrExtObj, storeNameArr)` -- create a new [Stream Aggregate](Stream-Aggregates). The `ftrExtObj = {type : 'ftrext', name : 'aggr1', featureSpace: fsp }` object has three parameters: `type='ftrext'`,`name` (string) and feature space `featureSpace` whose value is a feature space object.  Second parameter `storeNameArr` is an array of store names, where the stream aggregate will be registered.
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
	* @returns {module:qm.StreamAggr} Self. Values in the stream aggregator are changed as defined in the inner onUpdate function.
	*/
	//# exports.StreamAggr.prototype.onAdd = function (rec) {};
	JsDeclareFunction(onAdd);

	//!- `sa = sa.onUpdate(rec)` -- executes onUpdate function given an input record `rec` and returns self
	/**
	* Executes the function when a record in the store is updated.
	* @param {module:qmRecord} rec - The updated record given to the stream aggregator.
	* @returns {module:qm.StreamAggr} Self. Values in the stream aggregator are changed as defined in the inner onUpdate function.
	*/
	//# exports.StreamAggr.prototype.onUpdate = function (rec) {};
	JsDeclareFunction(onUpdate);

	//!- `sa = sa.onDelete(rec)` -- executes onDelete function given an input record `rec` and returns self
	/**
	* Executes the function when a record in the store is deleted.
	* @param {module:qm.Record} rec - The deleted record given to the stream aggregator.
	* @returns {module:qm.StreamAggr} Self. The values in the stream aggregator are changed as defined in the inner onDelete function.
	*/
	//# exports.StreamAggr.prototype.onDelete = function (rec) {};
	JsDeclareFunction(onDelete);
	
	//!- `objJSON = sa.saveJson(limit)` -- executes saveJson given an optional number parameter `limit`, whose meaning is specific to each type of stream aggregate
	/**
	* When executed it return a JSON object as defined by the user.
	* @param {number} [limit] - The meaning is specific to each type of stream aggregator. //TODO
	* @returns {Object} A JSON object as defined by the user.
	*/
	//# exports.StreamAggr.prototype.saveJson = function (limit) {};
	JsDeclareFunction(saveJson);

	//!- `fout = sa.save(fout)` -- executes save function given output stream `fout` as input. returns `fout`.
	JsDeclareFunction(save);
	//!- `sa = sa.load(fin)` -- executes load function given input stream `fin` as input. returns self.
	JsDeclareFunction(load);
	// IInt
	//!- `num = sa.getInt()` -- returns a number if sa implements the interface IInt
	JsDeclareFunction(getInteger);
	// IFlt
	//!- `num = sa.getFlt()` -- returns a number if sa implements the interface IFlt
	JsDeclareFunction(getFloat);
	// ITm
	//!- `num = sa.getTm()` -- returns a number if sa implements the interface ITm. The result is a windows timestamp (number of milliseconds since 1601)
	JsDeclareFunction(getTimestamp);

	// IFltVec
	//!- `num = sa.getFltLen()` -- returns a number (internal vector length) if sa implements the interface IFltVec.
	/**
	* Gets the length of the value vector.
	* @returns {number} The length of the value vector.
	*/
	//# exports.StreamAggr.prototype.getFloatLength = function () {};
	JsDeclareFunction(getFloatLength);

	//!- `num = sa.getFltAt(idx)` -- returns a number (element at index) if sa implements the interface IFltVec.
	/**
	* Returns the value of the float vector at a specific index.
	* @param {number} idx - The index.
	* @returns {number} The value of the float vector at position idx.
	*/
	//# exports.StreamAggr.prototype.getFloatAt = function (idx) {};
	JsDeclareFunction(getFloatAt);
	
	//!- `vec = sa.getFltV()` -- returns a dense vector if sa implements the interface IFltVec.
	/**
	* Gets the whole value vector of the buffer.
	* @returns {module:la.Vector} The vector containing the values of the buffer.
	*/
	//# exports.StreamAggr.prototype.getFloatVector = function () {};
	JsDeclareFunction(getFloatVector);

	// ITmVec
	//!- `num = sa.getTmLen()` -- returns a number (timestamp vector length) if sa implements the interface ITmVec.
	/**
	* Gets the length of the timestamp vector.
	* @returns {number} The length of the timestamp vector.
	*/
	//# exports.StreamAggr.prototype.getTimestampLength = function () {};
	JsDeclareFunction(getTimestampLength);

	//!- `num = sa.getTmAt(idx)` -- returns a number (windows timestamp at index) if sa implements the interface ITmVec.
	/**
	* Gets the timestamp on the specific location in the buffer.
	* @param {number} idx - The index.
	* @returns {number} The timestamp located on the idx position in the buffer.
	*/
	//# exports.StreamAggr.prototype.getTimestampAt = function (idx) {};
	JsDeclareFunction(getTimestampAt);

	//!- `vec = sa.getTmV()` -- returns a dense vector of windows timestamps if sa implements the interface ITmVec.
	/**
	* Gets the dense vector containing the windows timestamps.
	* @returns {module:la.Vector} The vector containing the window timestamps.
	*/
	//# exports.StreamAggr.prototype.getTimestampVector = function () {};
	JsDeclareFunction(getTimestampVector);
	
	// IFltTmIO
	//!- `num = sa.getInFlt()` -- returns a number (input value arriving in the buffer) if sa implements the interface IFltTmIO.
	/**
	* Gets the value of the newest record added to the buffer.
	* @returns {number} The value of the newest record in the buffer.
	*/
	//# exports.StreamAggr.prototype.getInFloat = function () {};
	JsDeclareFunction(getInFloat);

	//!- `num = sa.getInTm()` -- returns a number (windows timestamp arriving in the buffer) if sa implements the interface IFltTmIO.
	/**
	* Gets the timestamp of the newest record added to the buffer.
	* @returns {number} The timestamp given as the number of miliseconds since 01.01.1601, time: 00:00:00.0.
	*/
	//# exports.StreamAggr.prototype.getInTimestamp = function () {};
	JsDeclareFunction(getInTimestamp);
	
	//!- `vec = sa.getOutFltV()` -- returns a dense vector (values leaving the buffer) if sa implements the interface IFltTmIO.
	/**
	* Gets a vector containing the values that are leaving the buffer.
	* @returns {module:la.Vector} The vector containing the values that are leaving the buffer.
	*/
	//# exports.StreamAggr.prototype.getOutFloatVector = function () {};
	JsDeclareFunction(getOutFloatVector);

	//!- `vec = sa.getOutTmV()` -- returns a dense vector (windows timestamps leaving the bugger) if sa implements the interface IFltTmIO.
	/**
	* Gets a vector containing the timestamps that are leaving the buffer.
	* @returns {module:la.Vector} The vector containing the leaving timestamps.
	*/
	//# exports.StreamAggr.prototype.getOutTimestampVector = function () {};
	JsDeclareFunction(getOutTimestampVector);

	//!- `num = sa.getN()` -- returns a number of records in the input buffer if sa implements the interface IFltTmIO.
	/**
	* Gets the number of records in the buffer.
	* @returns {number} The number of records in the buffer.
	*/
	//# exports.StreamAggr.prototype.getNumberOfRecords = function () {};
	JsDeclareFunction(getNumberOfRecords);

	//!- `str = sa.name` -- returns the name (unique) of the stream aggregate
	/**
	* Returns the name of the stream aggregate.
	*/
	//# exports.StreamAggr.prototype.name = undefined;
	JsDeclareProperty(name);

	//!- `objJSON = sa.val` -- same as sa.saveJson(-1)
	/**
	* Returns a JSON object of the stream aggregate. Same as the method saveJson.
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

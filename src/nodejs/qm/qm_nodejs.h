#ifndef QMINER_QM_NODEJS
#define QMINER_QM_NODEJS

#include <node.h>
#include <node_object_wrap.h>
#include <qminer.h>
#include "../la/la_nodejs.h"
#include "../fs/fs_nodejs.h"
#include "../nodeutil.h"

///////////////////////////////
// NodeJs QMiner.
// A factory of base objects
// The only part of node framework: Init


/**
* Qminer module.
* @module qm
* @example 
* // import module
* var qm = require('qminer');
*/
class TNodeJsQm : public node::ObjectWrap {
public:
	// Node framework
	static void Init(v8::Handle<v8::Object> exports);	
	// TNodeJsRec needs this to select a template. TODO remove, see comment in v8::Local<v8::Object> TNodeJsRec::New(const TQm::TRec& Rec, const TInt& _Fq)
	static THash<TStr, TUInt> BaseFPathToId;
private:
	/**
	* Creates a directory structure.
	* @param {string} [configPath='qm.conf'] - The path to configuration file.
	* @param {boolean} [overwrite=false] - If you want to overwrite the configuration file.
	* @param {number} [portN=8080] - The number of the port. Currently not used.
	* @param {number} [cacheSize=1024] - Sets available memory for indexing (in MB).
	*/
	//# exports.config = function(configPath, overwrite, portN, cacheSize) {}
	JsDeclareFunction(config);

	/**
	* Creates an empty base.
	* @param {string} [configPath='qm.conf'] - Configuration file path.
	* @param {string} [schemaPath=''] - Schema file path.
	* @param {boolean} [clear=false] - Clear the existing db folder.
	* @returns {module:qm.Base}
	*/
	//# exports.create = function (configPath, schemaPath, clear) { return Object.create(require('qminer').Base.prototype); }
	JsDeclareFunction(create);

	/**
	* Opens a base.
	* @param {string} [configPath='qm.conf'] - The configuration file path.
	* @param {boolean} [readOnly=false] - Open in read only mode?
	* @returns {module:qm.Base}
	*/
	//# exports.open = function (configPath, readOnly) { return Object.create(require('qminer').Base.prototype); }
	JsDeclareFunction(open);
};

///////////////////////////////
// NodeJs QMiner Base

/**
* Base access modes.
* @readonly
* @enum {string}
*/
//# var baseModes = {
//#    create: 'create',
//#    createClean: 'createClean',
//#    open: 'open',
//#    openReadOnly: 'openReadOnly'
//# }

/**
* Base constructor parameter object
* @typedef {Object} BaseConstructorParam
* @property  {module:qm~baseModes} [BaseConstructorParam.mode='openReadOnly'] - Base access mode: 
* <br> create (sets up the db folder), 
* <br> createClean (cleans db folder and then sets it up), 
* <br> open (opens the db with read/write permissions), 
* <br> openReadOnly (opens the db in read only mode).
* @property  {number} [BaseConstructorParam.indexCache=1024] - The ammount of memory reserved for indexing (in MB).
* @property  {number} [BaseConstructorParam.storeCache=1024] - The ammount of memory reserved for store cache (in MB).
* @property  {string} [BaseConstructorParam.schemaPath=''] - The path to schema definition file.
* @property  {Object} [BaseConstructorParam.schema=[]] - Schema definition object.
* @property  {string} [BaseConstructorParam.dbPath='./db/'] - The path to db directory.
*/


/**
* Base
* @classdesc Represents the database and holds stores.
* @class
* @param {module:qm~BaseConstructorParam} paramObj - The base constructor parameter object
* @example
* // import qm module
* var qm = require('qminer');
* // factory based construction: create a base with the qm configuration file, without the predefined schemas, with overwrite = true
* var base = qm.create('qm.conf', "", true);
* base.close();
* // using a constructor, in open mode:
* var base2 = new qm.Base({mode: 'open'});
*/
//# exports.Base = function (paramObj) {};
class TNodeJsBase : public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	static v8::Persistent<v8::Function> Constructor;
public:
	static void Init(v8::Handle<v8::Object> Exports);
	static const TStr ClassId;
	// wrapped C++ object
	TWPt<TQm::TBase> Base;
	// C++ constructor
	TNodeJsBase(const TWPt<TQm::TBase>& Base_) : Base(Base_) { }
	TNodeJsBase(const TStr& DbPath, const TStr& SchemaFNm, const PJsonVal& Schema, const bool& Create, const bool& ForceCreate, const bool& ReadOnly, const TInt& IndexCache, const TInt& StoreCache);
private:	
	// TODO JSDOC
	// parses arguments, called by javascript constructor 
	static TNodeJsBase* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);
private:
	/**
	* Closes the database.
	* @returns {null}
	*/
	//# exports.Base.prototype.close = function () {}
	JsDeclareFunction(close);

	/**
	 * Returns the store with the specified name.
	 *
	 * @param {string} name - Name of the store.
	 * @returns {module:la.Store} The store.
	 */
	//# exports.Base.prototype.store = function (name) { return ''; }
	JsDeclareFunction(store);

	/**
	 * Returns a list of store descriptors.
	 *
	 * @returns {Object[]}
	 */
	//# exports.Base.prototype.getStoreList = function () { return [{storeId:'', storeName:'', storeRecords:'', fields: [], keys: [], joins: []}]; }
	JsDeclareFunction(getStoreList);
	/**
	* Creates a new store.
	* @param {Object} storeDef - The definition of the store(s)
	* @param {number} [storeSizeInMB = 1024] - The reserved size of the store(s).
	* @returns {(module:qm.Store | module:qm.Store[])} - Returns a store or an array of stores (if the schema definition was an array)
	*/
	//# exports.Base.prototype.createStore = function (storeDef, storeSizeInMB) { return storeDef instanceof Array ? [Object.create(require('qminer').Store.prototype)] : Object.create(require('qminer').Store.prototype) ;}
	JsDeclareFunction(createStore);

    //!- `rs = base.search(query)` -- execute `query` (Json) specified in [QMiner Query Language](Query Language) 
    //!   and returns a record set `rs` with results
	JsDeclareFunction(search);   
    //!- `base.gc()` -- start garbage collection to remove records outside time windows
	JsDeclareFunction(gc);
	//!- `sa = base.getStreamAggr(saName)` -- gets the stream aggregate `sa` given name (string).
	JsDeclareFunction(getStreamAggr);
	//!- `strArr = base.getStreamAggrNames()` -- gets the stream aggregate names of stream aggregates in the default stream aggregate base.
	JsDeclareFunction(getStreamAggrNames);	
	//!JSIMPLEMENT:src/qminer/qminer.js    
};

//! 
//! ### NodeJs Stream Aggregate
//! 
//! Stream aggregates are objects used for processing data streams - their main functionality includes four functions: onAdd, onUpdate, onDelte process a record, and saveJson which returns a JSON object that describes the aggregate's state.
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
	JsDeclareFunction(onAdd);
	//!- `sa = sa.onUpdate(rec)` -- executes onUpdate function given an input record `rec` and returns self
	JsDeclareFunction(onUpdate);
	//!- `sa = sa.onDelete(rec)` -- executes onDelete function given an input record `rec` and returns self
	JsDeclareFunction(onDelete);
	//!- `objJSON = sa.saveJson(limit)` -- executes saveJson given an optional number parameter `limit`, whose meaning is specific to each type of stream aggregate
	JsDeclareFunction(saveJson);
	//!- `fout = sa.save(fout)` -- executes save function given output stream `fout` as input. returns `fout`.
	JsDeclareFunction(save);
	//!- `sa = sa.load(fin)` -- executes load function given input stream `fin` as input. returns self.
	JsDeclareFunction(load);
	// IInt
	//!- `num = sa.getInt()` -- returns a number if sa implements the interface IInt
	JsDeclareFunction(getInt);
	// IFlt
	//!- `num = sa.getFlt()` -- returns a number if sa implements the interface IFlt
	JsDeclareFunction(getFlt);
	// ITm
	//!- `num = sa.getTm()` -- returns a number if sa implements the interface ITm. The result is a windows timestamp (number of milliseconds since 1601)
	JsDeclareFunction(getTm);
	// IFltVec
	//!- `num = sa.getFltLen()` -- returns a number (internal vector length) if sa implements the interface IFltVec.
	JsDeclareFunction(getFltLen);
	//!- `num = sa.getFltAt(idx)` -- returns a number (element at index) if sa implements the interface IFltVec.
	JsDeclareFunction(getFltAt);
	//!- `vec = sa.getFltV()` -- returns a dense vector if sa implements the interface IFltVec.
	JsDeclareFunction(getFltV);
	// ITmVec
	//!- `num = sa.getTmLen()` -- returns a number (timestamp vector length) if sa implements the interface ITmVec.
	JsDeclareFunction(getTmLen);
	//!- `num = sa.getTmAt(idx)` -- returns a number (windows timestamp at index) if sa implements the interface ITmVec.
	JsDeclareFunction(getTmAt);
	//!- `vec = sa.getTmV()` -- returns a dense vector of windows timestamps if sa implements the interface ITmVec.
	JsDeclareFunction(getTmV);
	// IFltTmIO
	//!- `num = sa.getInFlt()` -- returns a number (input value arriving in the buffer) if sa implements the interface IFltTmIO.
	JsDeclareFunction(getInFlt);
	//!- `num = sa.getInTm()` -- returns a number (windows timestamp arriving in the buffer) if sa implements the interface IFltTmIO.
	JsDeclareFunction(getInTm);
	//!- `vec = sa.getOutFltV()` -- returns a dense vector (values leaving the buffer) if sa implements the interface IFltTmIO.
	JsDeclareFunction(getOutFltV);
	//!- `vec = sa.getOutTmV()` -- returns a dense vector (windows timestamps leaving the bugger) if sa implements the interface IFltTmIO.
	JsDeclareFunction(getOutTmV);
	//!- `num = sa.getN()` -- returns a number of records in the input buffer if sa implements the interface IFltTmIO.
	JsDeclareFunction(getN);

	//!- `str = sa.name` -- returns the name (unique) of the stream aggregate
	JsDeclareProperty(name);
	//!- `objJSON = sa.val` -- same as sa.saveJson(-1)
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

///////////////////////////////
// NodeJs-Qminer-Store

/**
* Store
* @classdesc Represents the store object. TODO new constructor
* @class
* @param {module:qm.Base} base - The base where the store will be added.
* @param {module:qm~StoreConstructorParam} paramObj - The store schema object.
* @example
* // import qm module
* var qm = require('qminer');
* // factory based construction using base.createStore
* // TODO
* // using a constructor
* // TODO
*/
//# exports.Store = function (base, storeDef) {};
class TNodeJsStore : public node::ObjectWrap {
private:
	// Node framework
	static v8::Persistent<v8::Function> constructor;
public:	
	TWPt<TQm::TStore> Store;

	// Node framework 
	static void Init(v8::Handle<v8::Object> exports);
	// Wrapping C++ object
	static v8::Local<v8::Object> New(TWPt<TQm::TStore> _Store);
	// C++ constructors
	TNodeJsStore() { }
	TNodeJsStore(TWPt<TQm::TStore> _Store) : Store(_Store) { }
	// Node framework (constructor method)
	JsDeclareFunction(New);
	// Field accessors
	static v8::Local<v8::Value> Field(const TQm::TRec& Rec, const int FieldId);
	static v8::Local<v8::Value> Field(const TWPt<TQm::TStore>& Store, const uint64& RecId, const int FieldId);
private:
	//! 
	//! **Functions and properties:**
	//!
	//!- `rec = store.rec(recName)` -- get record named `recName`; 
	//!     returns `null` when no such record exists
	/**
	* Returns a record form the store.
	* @param {string} recName - Record name.
	* @returns {Object} Returns the record. If record doesn't exist, it returns null. //TODO
	*/
	//# exports.Store.prototype.rec = function (recName) {};
	JsDeclareFunction(rec);

	//!- `store = store.each(callback)` -- iterates through the store and executes the callback function `callback` on each record. Same record JavaScript wrapper is used for all callback; to save record, make a clone (`rec.$clone()`). Returns self. Examples:
	//!  - `store.each(function (rec) { console.log(JSON.stringify(rec)); })`
	//!  - `store.each(function (rec, idx) { console.log(JSON.stringify(rec) + ', ' + idx); })`
	/**
	* Executes a function on each record in store.
	* @param {function} callback - Function to be executed. It takes two parameters:
	* <br>rec - The current record.
	* <br>[idx] - The index of the current record.
	* @returns {module:qm.Store} Self.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a store with some people with fields Name and Gender
	* var store = //TODO
	* // change the gender of all records to "Extraterrestrial"
	* store.each(function (rec) { rec.Gender = "Extraterestrial"; });
	*/
	//# exports.Store.prototype.each = function (callback) {}
	JsDeclareFunction(each);

	//!- `arr = store.map(callback)` -- iterates through the store, applies callback function `callback` to each record and returns new array with the callback outputs. Same record JavaScript wrapper is used for all callback; to save record, make a clone (`rec.$clone()`). Examples:
	//!  - `arr = store.map(function (rec) { return JSON.stringify(rec); })`
	//!  - `arr = store.map(function (rec, idx) {  return JSON.stringify(rec) + ', ' + idx; })`
	/**
	* Creates an array of function outputs created from the store records.
	* @param {function} callback - Function that generates the array. It takes two parameters:
	* <br>rec - The current record.
	* <br>[idx] - The index of the current record.
	* @returns {Array<Object>} The array created by the callback function. //TODO
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a store with some people with fields Name and Gender
	* var store = //TODO
	* // make an array of recod names
	* var arr = store.map(function (rec) { return rec.Name; });
	*/
	//# exports.Store.prototype.map = function (callback) {}
	JsDeclareFunction(map);

	//!- `recId = store.add(rec)` -- add record `rec` to the store and return its ID `recId`
	/**
	* Adds a record to the store.
	* @param {Object} rec - The added record. //TODO
	* @returns {number} The ID of the added record.
	*/
	//# exports.Store.prototype.add = function (rec) {}
	JsDeclareFunction(add);

	//!- `rec = store.newRec(recordJson)` -- creates new record `rec` by (JSON) value `recordJson` (not added to the store)
	JsDeclareFunction(newRec);
	//!- `rs = store.newRecSet(idVec)` -- creates new record set from an integer vector record IDs `idVec` (type la.newIntVec);
	JsDeclareFunction(newRecSet);

	//!- `rs = store.sample(sampleSize)` -- create a record set containing a random 
	//!     sample of `sampleSize` records
	/**
	* Creates a record set containing random records from store.
	* @param {number} sampleSize - The size of the record set.
	* @returns {Array.<module:qm.Record>} Returns a record set containing a random record set.
	*/
	//# exports.Store.prototype.sample = function (sampleSize) {};
	JsDeclareFunction(sample);

	//!- `field = store.field(fieldName)` -- get details of field named `fieldName`
	/**
	* Gets the details of the selected field.
	* @param {string} fieldName - The name of the field.
	* @returns {Object} The JSON object containing the details of the field. //TODO
	*/
	//# exports.Store.prototype.field = function (fieldName) {}; 
	JsDeclareFunction(field);

	//!- `bool = store.isNumeric(fieldName)` -- returns true if the field is of numeric type
	/**
	* Checks if the field is of numeric type.
	* @param {string} fieldName - The checked field.
	* @returns {boolean} True, if the field is of numeric type. Otherwise, false.
	*/
	//# exports.Store.prototype.isNumeric = function (fieldName) {};
	JsDeclareFunction(isNumeric)

	//!- `bool = store.isString(fieldName)` -- returns true if the field is of String type
	/**
	* Checks if the field is of string type.
	* @param {string} fieldName - The checked field.
	* @returns {boolean} True, if the field is of the string type. Otherwise, false.
	*/
	//# exports.Store.prototype.isString = function (fieldName) {}; 
	JsDeclareFunction(isString)

	//!- `bool = store.isDate(fieldName)` -- returns true if the field is of type Date
	JsDeclareFunction(isDate)
	//!- `key = store.key(keyName)` -- get [index key](#index-key) named `keyName`
	/**
	* Returns the details of the selected key.
	* @param {string} keyName - The selected key.
	* @returns {Object} The JSON object containing the details of the key. //TODO
	*/
	//# exports.Store.prototype.key = function (keyName) {};
	JsDeclareFunction(key);

	////!- `store.addTrigger(trigger)` -- add `trigger` to the store triggers. Trigger is a JS object with three properties `onAdd`, `onUpdate`, `onDelete` whose values are callbacks
	//JsDeclareFunction(addTrigger); Deprecated - use new qm.sa(...) instead
	//!- `sa = store.getStreamAggr(saName)` -- returns a stream aggregate `sa` whose name is `saName`
	JsDeclareFunction(getStreamAggr);
	//!- `strArr = store.getStreamAggrNames()` -- returns the names of all stream aggregators listening on the store as an array of strings `strArr`
	JsDeclareFunction(getStreamAggrNames);
	//!- `objJSON = store.toJSON()` -- returns the store as a JSON
	/**
	* Returns the store as a JSON.
	* @returns {Object} The store as a JSON.
	*/
	//# exports.Store.prototype.toJSON = function () {};
	JsDeclareFunction(toJSON);

	//!- `store.clear()` -- deletes all records
	//!- `len = store.clear(num)` -- deletes the first `num` records and returns new length `len`
	/**
	* Deletes the records in the store.
	* @param {number} [num] - The number of deleted records.
	* @returns {number} The number of remaining records in the store.
	* @example
	* // delete all records in store
	* store.clear();	// returns 0
	* // deletes the first 10 records
	* store.clear(10);
	*/
	//# exports.Store.prototype.clear = function (num) {};
	JsDeclareFunction(clear);

	//!- `vec = store.getVec(fieldName)` -- gets the `fieldName` vector - the corresponding field type must be one-dimensional, e.g. float, int, string,...
	/**
	* Gives a vector containing the field value of each record.
	* @param {string} fieldName - The field where the data is taken from records.
	* @returns {module:la.Vector} The vector containing the field values of each record.
	*/
	//# exports.Store.prototype.getVec = function (fieldName) {};
	JsDeclareFunction(getVec);
	//!- `mat = store.getMat(fieldName)` -- gets the `fieldName` matrix - the corresponding field type must be float_v or num_sp_v
	JsDeclareFunction(getMat);
	//!- `val = store.cell(recId, fieldId)` -- if fieldId (int) corresponds to fieldName, this is equivalent to store[recId][fieldName]
	//!- `val = store.cell(recId, fieldName)` -- equivalent to store[recId][fieldName]
	JsDeclareFunction(cell);

	//!- `str = store.name` -- name of the store
	/**
	* Gives the name of the store.
	*/
	//# exports.Store.prototype.name = undefined;
	JsDeclareProperty(name);

	//!- `bool = store.empty` -- `bool = true` when store is empty
	/**
	* Checks if the store is empty.
	*/
	//# exports.Store.prototype.empty = undefined;
	JsDeclareProperty(empty);

	//!- `len = store.length` -- number of records in the store
	/**
	* Gives the number of records.
	*/
	//# exports.Store.prototype.length = 0;
	JsDeclareProperty(length);

	//!- `rs = store.recs` -- create a record set containing all the records from the store
	/**
	* Creates a record set containing all the records from the store.
	*/
	//# exports.Store.prototype.recs = undefined;
	JsDeclareProperty(recs);

	//!- `objArr = store.fields` -- array of all the field descriptor JSON objects
	/**
	* Gives an array of all field descriptor JSON objects.
	*/
	//# exports.Store.prototype.fields = undefinied;
	JsDeclareProperty(fields);

	//!- `objArr = store.joins` -- array of all the join names
	/**
	* Creates an array of all the join objects.
	*/
	//# exports.Store.prototype.joins = undefined;
	JsDeclareProperty(joins);

	//!- `objArr = store.keys` -- array of all the [index keys](#index-key) objects    
	/**
	* Creates an array of all the key objects.
	*/
	//# exports.Store.prototype.keys = undefined;
	JsDeclareProperty(keys);

	//!- `rec = store.first` -- first record from the store
	/**
	* Returns the first record of the store.
	*/
	//# exports.Store.prototype.first = undefined;
	JsDeclareProperty(first);

	//!- `rec = store.last` -- last record from the store
	/**
	* Returns the last record of the store.
	*/
	//# exports.Store.prototype.last = undefined;
	JsDeclareProperty(last);

	//!- `iter = store.forwardIter` -- returns iterator for iterating over the store from start to end
	/**
	* Returns an iterator for iterating over the store from start to end.
	*/
	//# exports.Store.prototype.forwardIter = undefined;
	JsDeclareProperty(forwardIter);

	//!- `iter = store.backwardIter` -- returns iterator for iterating over the store from end to start
	/**
	* Returns an iterator for iterating over the sotre form end to start.
	*/
	//# exports.Store.prototype.backwardIter = undefined;
	JsDeclareProperty(backwardIter);

	//!- `rec = store[recId]` -- get record with ID `recId`; 
	//!     returns `null` when no such record exists
	JsDeclIndexedProperty(indexId);	
	//!- `base = store.base` -- get store base; 
	JsDeclareProperty(base);
	//!JSIMPLEMENT:src/qminer/store.js
};

///////////////////////////////
// NodeJs QMiner Record
class TNodeJsRec: public node::ObjectWrap {
private:
	// Modified node framework: one record template per each base,storeId combination 
	static TVec<TVec<v8::Persistent<v8::Function> > > BaseStoreIdConstructor;
public:
	// Node framework 
	static void Init(const TWPt<TQm::TStore>& Store);
	// when reseting a db we have to clear the old record templates
	static void Clear(const int& BaseId);
	// Wrapping C++ object	
	static v8::Local<v8::Object> New(const TQm::TRec& Rec, const TInt& _Fq = 0);
	// C++ constructors
	TNodeJsRec(): Rec() {}
	TNodeJsRec(const TQm::TRec& _Rec): Rec(_Rec) {}
	TNodeJsRec(const TQm::TRec& _Rec, const TInt& _Fq) : Rec(_Rec), Fq(_Fq) {}
	// Node framework (constructor method)
	JsDeclareFunction(New);
public:
	// C++ wrapped object
	TQm::TRec Rec;	
	TInt Fq;

private:
	//!
	//! **Functions and properties:**
	//!
    //!- `rec2 = rec.$clone()` -- create a clone of JavaScript wrapper with same record inside
    JsDeclareFunction(clone);
    //!- `rec = rec.addJoin(joinName, joinRecord)` -- adds a join record `joinRecord` to join `jonName` (string). Returns self.
    //!- `rec = rec.addJoin(joinName, joinRecord, joinFrequency)` -- adds a join record `joinRecord` to join `jonName` (string) with join frequency `joinFrequency`. Returns self.
    JsDeclareFunction(addJoin);
    //!- `rec = rec.delJoin(joinName, joinRecord)` -- deletes join record `joinRecord` from join `joinName` (string). Returns self.
    //!- `rec = rec.delJoin(joinName, joinRecord, joinFrequency)` -- deletes join record `joinRecord` from join `joinName` (string) with join frequency `joinFrequency`. Return self.
    JsDeclareFunction(delJoin);
    //!- `objJSON = rec.toJSON()` -- provide json version of record, useful when calling JSON.stringify
    JsDeclareFunction(toJSON);

	//!- `recId = rec.$id` -- returns record ID
	JsDeclareProperty(id);
	//!- `recName = rec.$name` -- returns record name
	JsDeclareProperty(name);
	//!- `recFq = rec.$fq` -- returns record frequency (used for randomized joins)
	JsDeclareProperty(fq);
	//!- `recStore = rec.$store` -- returns record store
	JsDeclareProperty(store);
	//!- `rec['fieldName'] = val` -- sets the record's field `fieldName` to `val`. Equivalent: `rec.fieldName = val`.
	//!- `val = rec['fieldName']` -- gets the value `val` at field `fieldName`. Equivalent: `val = rec.fieldName`.
	JsDeclareSetProperty(getField, setField);
	//!- `rs = rec['joinName']` -- gets the record set if `joinName` is an index join. Equivalent: `rs = rec.joinName`. No setter currently.
	//!- `rec2 = rec['joinName']` -- gets the record `rec2` is the join `joinName` is a field join. Equivalent: `rec2 = rec.joinName`. No setter currently.
	JsDeclareProperty(join);
	JsDeclareProperty(sjoin);
};

///////////////////////////////
// NodeJs QMiner Record Set
class TNodeJsRecSet: public node::ObjectWrap {
private:
	// Node framework
	static v8::Persistent<v8::Function> constructor;
public:
	// Node framework 
	static void Init(v8::Handle<v8::Object> exports);
	// Wrapping C++ object	
	static v8::Local<v8::Object> New();
	static v8::Local<v8::Object> New(const TQm::PRecSet& Rec);
	// C++ constructors
	TNodeJsRecSet() {}
	TNodeJsRecSet(const TQm::PRecSet& _RecSet) : RecSet(_RecSet) {}
	// Node framework (constructor method)
	JsDeclareFunction(New);
public:
	// C++ wrapped object
	TQm::PRecSet RecSet;

private:
	//! 
	//! **Functions and properties:**
	//!   
	//!- `rs2 = rs.clone()` -- creates new instance of record set
	JsDeclareFunction(clone);
	//!- `rs2 = rs.join(joinName)` -- executes a join `joinName` on the records in the set, result is another record set `rs2`.
	//!- `rs2 = rs.join(joinName, sampleSize)` -- executes a join `joinName` on a sample of `sampleSize` records in the set, result is another record set `rs2`.
	JsDeclareFunction(join);
	//!- `aggrsJSON = rs.aggr()` -- returns an object where keys are aggregate names and values are JSON serialized aggregate values of all the aggregates contained in the records set
	//!- `aggr = rs.aggr(aggrQueryJSON)` -- computes the aggregates based on the `aggrQueryJSON` parameter JSON object. If only one aggregate is involved and an array of JSON objects when more than one are returned.
	JsDeclareFunction(aggr);
	//!- `rs = rs.trunc(limit_num)` -- truncate to first `limit_num` record and return self.
	//!- `rs = rs.trunc(limit_num, offset_num)` -- truncate to `limit_num` record starting with `offset_num` and return self.
	JsDeclareFunction(trunc);
	//!- `rs2 = rs.sample(num)` -- create new record set by randomly sampling `num` records.
	JsDeclareFunction(sample);
	//!- `rs = rs.shuffle(seed)` -- shuffle order using random integer seed `seed`. Returns self.
	JsDeclareFunction(shuffle);
	//!- `rs = rs.reverse()` -- reverse record order. Returns self.
	JsDeclareFunction(reverse);
	//!- `rs = rs.sortById(asc)` -- sort records according to record id; if `asc > 0` sorted in ascending order. Returns self.
	JsDeclareFunction(sortById);
	//!- `rs = rs.sortByFq(asc)` -- sort records according to weight; if `asc > 0` sorted in ascending order. Returns self.
	JsDeclareFunction(sortByFq);
	//!- `rs = rs.sortByField(fieldName, asc)` -- sort records according to value of field `fieldName`; if `asc > 0` sorted in ascending order (default is desc). Returns self.
	JsDeclareFunction(sortByField);
	//!- `rs = rs.sort(comparatorCallback)` -- sort records according to `comparator` callback. Example: rs.sort(function(rec,rec2) {return rec.Val < rec2.Val;} ) sorts rs in ascending order (field Val is assumed to be a num). Returns self.
	JsDeclareFunction(sort);
	//!- `rs = rs.filterById(minId, maxId)` -- keeps only records with ids between `minId` and `maxId`. Returns self.
	JsDeclareFunction(filterById);
	//!- `rs = rs.filterByFq(minFq, maxFq)` -- keeps only records with weight between `minFq` and `maxFq`. Returns self.
	JsDeclareFunction(filterByFq);
	//!- `rs = rs.filterByField(fieldName, minVal, maxVal)` -- keeps only records with numeric value of field `fieldName` between `minVal` and `maxVal`. Returns self.
	//!- `rs = rs.filterByField(fieldName, minTm, maxTm)` -- keeps only records with value of time field `fieldName` between `minVal` and `maxVal`. Returns self.
	//!- `rs = rs.filterByField(fieldName, str)` -- keeps only records with string value of field `fieldName` equal to `str`. Returns self.
	JsDeclareFunction(filterByField);
	//!- `rs = rs.filter(filterCallback)` -- keeps only records that pass `filterCallback` function. Returns self.
	JsDeclareFunction(filter);
	//!- `rsArr = rs.split(splitterCallback)` -- split records according to `splitter` callback. Example: rs.split(function(rec,rec2) {return (rec2.Val - rec2.Val) > 10;} ) splits rs in whenever the value of field Val increases for more than 10. Result is an array of record sets. 
	JsDeclareFunction(split);
	//!- `rs = rs.deleteRecs(rs2)` -- delete from `rs` records that are also in `rs2`. Returns self.
	JsDeclareFunction(deleteRecs);
	//!- `objsJSON = rs.toJSON()` -- provide json version of record set, useful when calling JSON.stringify
	JsDeclareFunction(toJSON);
	//!- `rs = rs.each(callback)` -- iterates through the record set and executes the callback function `callback` on each element. Same record JavaScript wrapper is used for all callback; to save record, make a clone (`rec.$clone()`). Returns self. Examples:
	//!  - `rs.each(function (rec) { console.log(JSON.stringify(rec)); })`
	//!  - `rs.each(function (rec, idx) { console.log(JSON.stringify(rec) + ', ' + idx); })`
	JsDeclareFunction(each);
	//!- `arr = rs.map(callback)` -- iterates through the record set, applies callback function `callback` to each element and returns new array with the callback outputs. Same record JavaScript wrapper is used for all callback; to save record, make a clone (`rec.$clone()`). Examples:
	//!  - `arr = rs.map(function (rec) { return JSON.stringify(rec); })`
	//!  - `arr = rs.map(function (rec, idx) {  return JSON.stringify(rec) + ', ' + idx; })`
	JsDeclareFunction(map);
	//!- `rs3 = rs.setintersect(rs2)` -- returns the intersection (record set) `rs3` between two record sets `rs` and `rs2`, which should point to the same store.
	JsDeclareFunction(setintersect);
	//!- `rs3 = rs.setunion(rs2)` -- returns the union (record set) `rs3` between two record sets `rs` and `rs2`, which should point to the same store.
	JsDeclareFunction(setunion);
	//!- `rs3 = rs.setdiff(rs2)` -- returns the set difference (record set) `rs3`=`rs`\`rs2`  between two record sets `rs` and `rs1`, which should point to the same store.
	JsDeclareFunction(setdiff);
	//!- `vec = rs.getVec(fieldName)` -- gets the `fieldName` vector - the corresponding field type must be one-dimensional, e.g. float, int, string,...
	JsDeclareFunction(getVec);
	//!- `vec = rs.getMat(fieldName)` -- gets the `fieldName` matrix - the corresponding field type must be float_v or num_sp_v
	JsDeclareFunction(getMat);
	
	//!- `storeName = rs.store` -- store of the records
	JsDeclareProperty(store);
	//!- `len = rs.length` -- number of records in the set
	JsDeclareProperty(length);
	//!- `bool = rs.empty` -- `bool = true` when record set is empty
	JsDeclareProperty(empty);
	//!- `bool =  rs.weighted` -- `bool = true` when records in the set are assigned weights
	JsDeclareProperty(weighted);
	//!- `rec = rs[n]` -- return n-th record from the record set
	JsDeclIndexedProperty(indexId);
};

///////////////////////////////
// NodeJs QMiner Store Iterator
//! 
//! ### Store iterator
//! 
class TNodeJsStoreIter: public node::ObjectWrap {
private:
	// Node framework
	static v8::Persistent<v8::Function> constructor;
public:
	// Node framework 
	static void Init(v8::Handle<v8::Object> exports);
	// Wrapping C++ object	
	static v8::Local<v8::Object> New();
	static v8::Local<v8::Object> New(const TWPt<TQm::TStore>& Store, const TQm::PStoreIter& Iter);
	// C++ constructors
	TNodeJsStoreIter() : JsRec(nullptr) {}
	TNodeJsStoreIter(const TWPt<TQm::TStore>& _Store, const TQm::PStoreIter& _Iter) : Store(_Store), Iter(_Iter), JsRec(nullptr) {}
	// Node framework (constructor method)
	JsDeclareFunction(New);
	
public:
	// C++ wrapped object
	TWPt<TQm::TStore> Store;	
	TQm::PStoreIter Iter;
    // placeholder for last object
	v8::Persistent<v8::Object> RecObj;
	TNodeJsRec* JsRec;
    // delete placeholder
    ~TNodeJsStoreIter() { RecObj.Reset(); }
	
    //!
	//! **Functions and properties:**
	//!   
	//!- `bool = iter.next()` -- moves to the next record or returns false if no record left; must be called at least once before `iter.rec` is available
	JsDeclareFunction(next);
	//!- `store = iter.store` -- get the store
	JsDeclareProperty(store);
	//!- `rec = iter.rec` -- get current record; reuses JavaScript record wrapper, need to call `rec.$clone()` on it to if there is any wish to store intermediate records.
	JsDeclareProperty(rec);
};

///////////////////////////////
// NodeJs QMiner Record Filter
class TJsRecFilter {
private:
	TWPt<TQm::TStore> Store;
	// Callbacks
	v8::Persistent<v8::Function> Callback;

public:
	~TJsRecFilter(){
		Callback.Reset();
	}
	TJsRecFilter(TWPt<TQm::TStore> _Store, v8::Handle<v8::Function> _Callback) : Store(_Store) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);
		// set persistent object
		Callback.Reset(Isolate, _Callback);
	}
	
	bool operator()(const TUInt64IntKd& RecIdWgt) const;
};

///////////////////////////////
// NodeJs QMiner Record Filter (record splitter, record comparator)
class TJsRecPairFilter {
private:
	TWPt<TQm::TStore> Store;
	// Callbacks
    v8::Persistent<v8::Function> Callback;
public:
	~TJsRecPairFilter(){
		Callback.Reset();
	}
	TJsRecPairFilter(TWPt<TQm::TStore> _Store, v8::Handle<v8::Function> _Callback) : Store(_Store) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);
		// set persistent object
		Callback.Reset(Isolate, _Callback);
	}

	bool operator()(const TUInt64IntKd& RecIdWgt1, const TUInt64IntKd& RecIdWgt2) const;
};

///////////////////////////////
// NodeJs QMiner Index Key
//!
//! ### Index key
//!
class TNodeJsIndexKey: public node::ObjectWrap {
private:
    // Node framework
    static v8::Persistent<v8::Function> constructor;
public:
    // Node framework
    static void Init(v8::Handle<v8::Object> exports);
    // Wrapping C++ object
    static v8::Local<v8::Object> New();
    static v8::Local<v8::Object> New(const TWPt<TQm::TStore>& _Store, const TQm::TIndexKey& _IndexKey);
    // C++ constructors
    TNodeJsIndexKey() {}
    TNodeJsIndexKey(const TWPt<TQm::TStore>& _Store, const TQm::TIndexKey& _IndexKey):
        Store(_Store), IndexKey(_IndexKey) { }
    // Node framework (constructor method)
    JsDeclareFunction(New);
    
public:
    // C++ wrapped object
    TWPt<TQm::TStore> Store;
    TQm::TIndexKey IndexKey;
    
public:
    //!
	//! **Functions and properties:**
	//!   
	//!- `store = key.store` -- gets the key's store
	JsDeclareProperty(store);
	//!- `keyName = key.name` -- gets the key's name
	JsDeclareProperty(name);
	//!- `strArr = key.voc` -- gets the array of words (as strings) in the vocabulary
	JsDeclareProperty(voc);
	//!- `strArr = key.fq` -- gets the array of weights (as strings) in the vocabulary
	JsDeclareProperty(fq);
};


///////////////////////////////////////////////
/// Javscript Function Feature Extractor.
//-
//- ## Javascript Feature Extractor
//-
class TNodeJsFuncFtrExt : public TQm::TFtrExt {
private:
	// private constructor
	TNodeJsFuncFtrExt(const TWPt<TQm::TBase>& Base, const PJsonVal& ParamVal, const v8::Handle<v8::Function> _Fun, v8::Isolate* Isolate);

	~TNodeJsFuncFtrExt() { Fun.Reset(); }
public:
	// public smart pointer
	static TQm::PFtrExt NewFtrExt(const TWPt<TQm::TBase>& Base, const PJsonVal& ParamVal, const v8::Handle<v8::Function>& Fun, v8::Isolate* Isolate);
// Core functionality
private:
	// Core part
	TInt Dim;
	TStr Name;
	
	v8::Persistent<v8::Function> Fun;

	double ExecuteFunc(const TQm::TRec& FtrRec) const {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);
		v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, Fun);
		return TNodeJsUtil::ExecuteFlt(Callback, TNodeJsRec::New(FtrRec));
	}

	void ExecuteFuncVec(const TQm::TRec& FtrRec, TFltV& Vec) const {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);

		v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, Fun);
		v8::Handle<v8::Value> Argv[1] = { TNodeJsRec::New(FtrRec) };
		v8::Handle<v8::Value> RetVal = Callback->Call(Isolate->GetCurrentContext()->Global(), 1, Argv);

		// Cast as FltV and copy result
		v8::Handle<v8::Object> RetValObj = v8::Handle<v8::Object>::Cast(RetVal);

		QmAssertR(TNodeJsUtil::IsClass(RetValObj, TNodeJsFltV::GetClassId()), "TJsFuncFtrExt::ExecuteFuncVec callback should return a dense vector (same type as la.newVec()).");

    	v8::Local<v8::External> WrappedObject = v8::Local<v8::External>::Cast(RetValObj->GetInternalField(0));
		// cast it to js vector and copy internal vector
		TNodeJsFltV* JsVec = static_cast<TNodeJsFltV*>(WrappedObject->Value());

		Vec = JsVec->Vec;
	}
public:
// Feature extractor API
private:
	TNodeJsFuncFtrExt(const TWPt<TQm::TBase>& Base, const PJsonVal& ParamVal); // will throw exception (saving, loading not supported)
	TNodeJsFuncFtrExt(const TWPt<TQm::TBase>& Base, TSIn& SIn); // will throw exception (saving, loading not supported)
public:
	static TQm::PFtrExt New(const TWPt<TQm::TBase>& Base, const PJsonVal& ParamVal); // will throw exception (saving, loading not supported)
	static TQm::PFtrExt Load(const TWPt<TQm::TBase>& Base, TSIn& SIn); // will throw exception (saving, loading not supported)
	void Save(TSOut& SOut) const;

	TStr GetNm() const { return Name; }
	int GetDim() const { return Dim; }
	TStr GetFtr(const int& FtrN) const { return TStr::Fmt("%s[%d]", GetNm().CStr(), FtrN) ; }

	void Clr() { };
	bool Update(const TQm::TRec& Rec) { return false; }
	void AddSpV(const TQm::TRec& Rec, TIntFltKdV& SpV, int& Offset) const;
	void AddFullV(const TQm::TRec& Rec, TFltV& FullV, int& Offset) const;

	void InvFullV(const TFltV& FullV, int& Offset, TFltV& InvV) const {
		throw TExcept::New("Not implemented yet!", "TJsFuncFtrExt::InvFullV");
	}

	// flat feature extraction
	void ExtractFltV(const TQm::TRec& FtrRec, TFltV& FltV) const;

	// feature extractor type name
	static TStr GetType() { return "jsfunc"; }
};

///////////////////////////////
// NodeJs QMiner Feature Space
class TNodeJsFtrSpace : public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	// Node framework
	static v8::Persistent<v8::Function> constructor;

	TQm::PFtrSpace FtrSpace;

	TNodeJsFtrSpace(const TQm::PFtrSpace& FtrSpace);
	TNodeJsFtrSpace(const TWPt<TQm::TBase> Base, TSIn& SIn);

	static v8::Local<v8::Object> WrapInst(const v8::Local<v8::Object> Obj, const TQm::PFtrSpace& FtrSpace);
	static v8::Local<v8::Object> WrapInst(const v8::Local<v8::Object> Obj, const TWPt<TQm::TBase> Base, TSIn& SIn);

public:
	static v8::Local<v8::Object> New(const TQm::PFtrSpace& FtrSpace);
	static v8::Local<v8::Object> New(const TWPt<TQm::TBase> Base, TSIn& SIn);

	// Node framework
	static void Init(v8::Handle<v8::Object> exports);

	TQm::PFtrSpace GetFtrSpace() { return FtrSpace; }
	//!
	//! **Constructor:**
	//!
	//!- `fsp = new qm.FeatureSpace(base, fin)` -- construct a new feature space by providing the base and input stream object
	//!- `fsp = new qm.FeatureSpace(base, params)` -- construct a new feature space by providing the base and parameter JSON object
	JsDeclareFunction(New);
	//!
	//! **Functions and properties:**
	//!
    //!- `num = fsp.dim` -- dimensionality of feature space
    JsDeclareProperty(dim);
    //!- `num_array = fsp.dims` -- dimensionality of feature space for each of the internal feature extarctors
    JsDeclareProperty(dims);
    //!- `fout = fsp.save(fout)` -- serialize feature space to `fout` output stream. Returns `fout`.
    JsDeclareFunction(save);

	//!- `fsp = fsp.add(objJson)` -- add a feature extractor parametrized by `objJson`
	JsDeclareFunction(add);

    //!- `fsp = fsp.updateRecord(rec)` -- update feature space definitions and extractors
    //!     by exposing them to record `rec`. Returns self. For example, this can update the vocabulary
    //!     used by bag-of-words extractor by taking into account new text.
	JsDeclareFunction(updateRecord);
    //!- `fsp = fsp.updateRecords(rs)` -- update feature space definitions and extractors
    //!     by exposing them to records from record set `rs`. Returns self. For example, this can update
    //!     the vocabulary used by bag-of-words extractor by taking into account new text.
	JsDeclareFunction(updateRecords);
	//!- `spVec = fsp.ftrSpVec(rec)` -- extracts sparse feature vector `spVec` from record `rec`
    JsDeclareFunction(ftrSpVec);
    //!- `vec = fsp.ftrVec(rec)` -- extracts feature vector `vec` from record  `rec`
    JsDeclareFunction(ftrVec);
    //!- `vec = fsp.invFtrVec(ftrVec)` -- performs the inverse operation of ftrVec, returns the results in
    //!- 	an array
	JsDeclareFunction(invFtrVec);
	//!- `val = fsp.invFtrVec(ftrIdx, val)` -- inverts a single feature using the feature
	//!- 	extractor on index `ftrIdx`
	JsDeclareFunction(invFtr);

    //!- `spMat = fsp.ftrSpColMat(rs)` -- extracts sparse feature vectors from
    //!     record set `rs` and returns them as columns in a sparse matrix `spMat`.
	JsDeclareFunction(ftrSpColMat);
    //!- `mat = fsp.ftrColMat(rs)` -- extracts feature vectors from
    //!     record set `rs` and returns them as columns in a matrix `mat`.
    JsDeclareFunction(ftrColMat);

	//!- `name = fsp.getFtrExtractor(ftrExtractor)` -- returns the name `name` (string) of `ftrExtractor`-th feature extractor in feature space `fsp`
	JsDeclareFunction(getFtrExtractor);
	//!- `ftrName = fsp.getFtr(idx)` -- returns the name `ftrName` (string) of `idx`-th feature in feature space `fsp`
	JsDeclareFunction(getFtr);
    //!- `vec = fsp.getFtrDist()` -- returns a vector with distribution over the features
    //!- `vec = fsp.getFtrDist(ftrExtractor)` -- returns a vector with distribution over the features for feature extractor ID `ftrExtractor`
    JsDeclareFunction(getFtrDist);
    //!- `out_vec = fsp.filter(in_vec, ftrExtractor)` -- filter the vector to keep only elements from the feature extractor ID `ftrExtractor`
    //!- `out_vec = fsp.filter(in_vec, ftrExtractor, keepOffset)` -- filter the vector to keep only elements from the feature extractor ID `ftrExtractor`.
    //!     If `keepOffset` == `true`, then original feature ID offset is kept, otherwise the first feature of `ftrExtractor` starts with position 0.
    JsDeclareFunction(filter);

	//!- `strArr = fsp.extractStrings(rec)` -- use feature extractors to extract string
    //!     features from record `rec` (e.g. words from string fields); results are returned
    //!     as a string array
    JsDeclareFunction(extractStrings);

private:
    static TQm::PFtrExt NewFtrExtFromFunc(const TWPt<TQm::TBase>& Base, v8::Local<v8::Object>& Settings, v8::Isolate* Isolate) {
    	PJsonVal ParamVal = TNodeJsUtil::GetObjProps(Settings);
    	v8::Handle<v8::Function> Func = v8::Handle<v8::Function>::Cast(Settings->Get(v8::String::NewFromUtf8(Isolate, "fun")));
    	return TNodeJsFuncFtrExt::NewFtrExt(Base, ParamVal, Func, Isolate);
    }
};

#endif

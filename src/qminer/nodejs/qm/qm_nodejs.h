#ifndef QMINER_QM_NODEJS
#define QMINER_QM_NODEJS

#define BUILDING_NODE_EXTENSION

#include <node.h>
#include <node_object_wrap.h>
#include "qminer.h"
#include "utils.h"

///////////////////////////////
// NodeJs-Qminer
// A factory of base objects
class TNodeJsQm : public node::ObjectWrap {
public:
	static void Init(v8::Handle<v8::Object> exports);
public:
	//# 
	//# **Functions and properties:**
	//# 
	//#- `base = qm.create(schemaPath, confPath)` -- TODO
	JsDeclareFunction(create);
	//#- `base = qm.open(confPath)` -- TODO
	JsDeclareFunction(open);	
};

///////////////////////////////
// NodeJs-Qminer-Base
// 
class TNodeJsBase : public node::ObjectWrap {
public:
	static void Init(v8::Handle<v8::Object> exports);
	static v8::Local<v8::Object> New(TWPt<TQm::TBase> _Base);
public:
	TNodeJsBase() { }
	TNodeJsBase(TWPt<TQm::TBase> _Base) : Base(_Base) { }
public:
	TWPt<TQm::TBase> Base;
public:
	JsDeclareFunction(New);
public:
	//# 
	//# **Functions and properties:**
	//# 
	//#- `store = base.store(close)` -- TODO
	JsDeclareFunction(close);
    //#- `store = base.store(storeName)` -- store with name `storeName`; `store = null` when no such store
	JsDeclareFunction(store);
    //#- `strArr = base.getStoreList()` -- an array of strings listing all existing stores
	JsDeclareFunction(getStoreList);
    //#- `base.createStore(storeDef)` -- create new store(s) based on given `storeDef` (Json) [definition](Store Definition)
    //#- `base.createStore(storeDef, storeSizeInMB)` -- create new store(s) based on given `storeDef` (Json) [definition](Store Definition)
	JsDeclareFunction(createStore);
    //#- `rs = base.search(query)` -- execute `query` (Json) specified in [QMiner Query Language](Query Language) 
    //#   and returns a record set `rs` with results
	JsDeclareFunction(search);   
    //#- `base.gc()` -- start garbage collection to remove records outside time windows
	JsDeclareFunction(gc);
	//#- `sa = base.newStreamAggr(paramJSON)` -- create a new [Stream Aggregate](Stream-Aggregates) object `sa`. The constructor parameters are stored in `paramJSON` object. `paramJSON` must contain field `type` which defines the type of the aggregate.
	//#- `sa = base.newStreamAggr(paramJSON, storeName)` -- create a new [Stream Aggregate](Stream-Aggregates) object `sa`. The constructor parameters are stored in `paramJSON` object. `paramJSON` must contain field `type` which defines the type of the aggregate. Second parameter `storeName` is used to register the stream aggregate for events on the appropriate store.
	//#- `sa = base.newStreamAggr(paramJSON, storeNameArr)` -- create a new [Stream Aggregate](Stream-Aggregates) object `sa`. The constructor parameters are stored in `paramJSON` object. `paramJSON` must contain field `type` which defines the type of the aggregate. Second parameter `storeNameArr` is an array of store names, where the stream aggregate will be registered.
	//#- `sa = base.newStreamAggr(funObj)` -- create a new [Stream Aggregate](Stream-Aggregates). The function object `funObj` defines the aggregate name and four callbacks: onAdd (takes record as input), onUpdate (takes record as input), onDelete (takes record as input) and saveJson (takes one numeric parameter - limit) callbacks. An example: `funObj = new function () {this.name = 'aggr1'; this.onAdd = function (rec) { }; this.onUpdate = function (rec) { }; this.onDelete = function (rec) { };  this.saveJson = function (limit) { return {}; } }`.
	//#- `sa = base.newStreamAggr(funObj, storeName)` -- create a new [Stream Aggregate](Stream-Aggregates). The function object `funObj` defines the aggregate name and four callbacks: onAdd (takes record as input), onUpdate (takes record as input), onDelete (takes record as input) and saveJson (takes one numeric parameter - limit) callbacks. An example: `funObj = new function () {this.name = 'aggr1'; this.onAdd = function (rec) { }; this.onUpdate = function (rec) { }; this.onDelete = function (rec) { };  this.saveJson = function (limit) { return {}; } }`.  Second parameter `storeName` is used to register the stream aggregate for events on the appropriate store.
	//#- `sa = base.newStreamAggr(funObj, storeNameArr)` -- create a new [Stream Aggregate](Stream-Aggregates). The function object `funObj` defines the aggregate name and four callbacks: onAdd (takes record as input), onUpdate (takes record as input), onDelete (takes record as input) and saveJson (takes one numeric parameter - limit) callbacks. An example: `funObj = new function () {this.name = 'aggr1'; this.onAdd = function (rec) { }; this.onUpdate = function (rec) { }; this.onDelete = function (rec) { };  this.saveJson = function (limit) { return {}; } }`.  Second parameter `storeNameArr` is an array of store names, where the stream aggregate will be registered.
	//#- `sa = base.newStreamAggr(ftrExtObj)` -- create a new [Stream Aggregate](Stream-Aggregates). The `ftrExtObj = {type : 'ftrext', name : 'aggr1', featureSpace: fsp }` object has three parameters: `type='ftrext'`,`name` (string) and feature space `featureSpace` whose value is a feature space object.
	//#- `sa = base.newStreamAggr(ftrExtObj, storeName)` -- create a new [Stream Aggregate](Stream-Aggregates). The `ftrExtObj = {type : 'ftrext', name : 'aggr1', featureSpace: fsp }` object has three parameters: `type='ftrext'`,`name` (string) and feature space `featureSpace` whose value is a feature space object.  Second parameter `storeName` is used to register the stream aggregate for events on the appropriate store.
	//#- `sa = base.newStreamAggr(ftrExtObj, storeNameArr)` -- create a new [Stream Aggregate](Stream-Aggregates). The `ftrExtObj = {type : 'ftrext', name : 'aggr1', featureSpace: fsp }` object has three parameters: `type='ftrext'`,`name` (string) and feature space `featureSpace` whose value is a feature space object.  Second parameter `storeNameArr` is an array of store names, where the stream aggregate will be registered.
	JsDeclareFunction(newStreamAggr);
	//#- `sa = base.getStreamAggr(saName)` -- gets the stream aggregate `sa` given name (string).
	JsDeclareFunction(getStreamAggr);
	//#- `strArr = base.getStreamAggrNames()` -- gets the stream aggregate names of stream aggregates in the default stream aggregate base.
	JsDeclareFunction(getStreamAggrNames);	
	//#JSIMPLEMENT:src/qminer/qminer.js    

private:
	static v8::Persistent<v8::Function> constructor;

};

///////////////////////////////
// NodeJs-Qminer-Rec
class TNodeJsRec: public node::ObjectWrap {
public:
	TQm::TRec Rec;

public:
	static void Init(v8::Handle<v8::Object> exports);
	static v8::Local<v8::Object> New(const TQm::TRec& Rec);
public:
	TNodeJsRec(): Rec() {}
	TNodeJsRec(const TQm::TRec& _Rec): Rec(_Rec) {}

	static TNodeJsRec* GetJsRec(v8::Local<v8::Object> RecObj);
private:

	//#
	//# **Functions and properties:**
	//#
    //#- `recId = rec.$id` -- returns record ID
    JsDeclareProperty(id);
    //#- `recName = rec.$name` -- returns record name
    JsDeclareProperty(name);
    //#- `recFq = rec.$fq` -- returns record frequency (used for randomized joins)
	JsDeclareProperty(fq);
	//#- `recStore = rec.$store` -- returns record store
	JsDeclareProperty(store);
    //#- `rec['fieldName'] = val` -- sets the record's field `fieldName` to `val`. Equivalent: `rec.fieldName = val`.
	//#- `val = rec['fieldName']` -- gets the value `val` at field `fieldName`. Equivalent: `val = rec.fieldName`.
	JsDeclareSetProperty(getField, setField);
    //#- `rs = rec['joinName']` -- gets the record set if `joinName` is an index join. Equivalent: `rs = rec.joinName`. No setter currently.
	//#- `rec2 = rec['joinName']` -- gets the record `rec2` is the join `joinName` is a field join. Equivalent: `rec2 = rec.joinName`. No setter currently.
	JsDeclareProperty(join);
	JsDeclareProperty(sjoin);
    //#- `rec2 = rec.$clone()` -- create a clone of JavaScript wrapper with same record inside
    JsDeclareFunction(clone);
    //#- `rec = rec.addJoin(joinName, joinRecord)` -- adds a join record `joinRecord` to join `jonName` (string). Returns self.
    //#- `rec = rec.addJoin(joinName, joinRecord, joinFrequency)` -- adds a join record `joinRecord` to join `jonName` (string) with join frequency `joinFrequency`. Returns self.
    JsDeclareFunction(addJoin);
    //#- `rec = rec.delJoin(joinName, joinRecord)` -- deletes join record `joinRecord` from join `joinName` (string). Returns self.
    //#- `rec = rec.delJoin(joinName, joinRecord, joinFrequency)` -- deletes join record `joinRecord` from join `joinName` (string) with join frequency `joinFrequency`. Return self.
    JsDeclareFunction(delJoin);
    //#- `objJSON = rec.toJSON()` -- provide json version of record, useful when calling JSON.stringify
    JsDeclareFunction(toJSON);

private:
	static v8::Persistent<v8::Function> constructor;
};

///////////////////////////////
// NodeJs-Qminer-Record-Set
class TNodeJsRecSet: public node::ObjectWrap {
public:
	TQm::PRecSet RecSet;
public:
	static void Init(v8::Handle<v8::Object> exports);
	static v8::Local<v8::Object> New(const TQm::PRecSet& Rec);
public:
	TNodeJsRecSet(const TQm::PRecSet& _RecSet) : RecSet(_RecSet) {}
	static TNodeJsRecSet* GetJsRecSet(v8::Local<v8::Object> RecObj);


private:
	static v8::Persistent<v8::Function> constructor;
};

///////////////////////////////
// NodeJs-Qminer-Store
class TNodeJsStore : public node::ObjectWrap {
public:
	static void Init(v8::Handle<v8::Object> exports);
	static v8::Local<v8::Object> New(TWPt<TQm::TStore> _Store, TWPt<TQm::TBase> _Base);
public:
	TNodeJsStore() { }
	TNodeJsStore(TWPt<TQm::TStore> _Store, TWPt<TQm::TBase> _Base) : Store(_Store), Base(_Base) { }
public:
	JsDeclareFunction(New);
private:
	//# 
	//# **Functions and properties:**
	//#     
	//#- `str = store.name` -- name of the store
	JsDeclareProperty(name);
	//#- `bool = store.empty` -- `bool = true` when store is empty
	JsDeclareProperty(empty);
	//#- `len = store.length` -- number of records in the store
	JsDeclareProperty(length);
	//#- `rs = store.recs` -- create a record set containing all the records from the store
	JsDeclareProperty(recs);
	//#- `objArr = store.fields` -- array of all the field descriptor JSON objects
	JsDeclareProperty(fields);
	//#- `objArr = store.joins` -- array of all the join names
	JsDeclareProperty(joins);
	//#- `objArr = store.keys` -- array of all the [index keys](#index-key) objects    
	JsDeclareProperty(keys);
	//#- `rec = store.first` -- first record from the store
	JsDeclareProperty(first);
	//#- `rec = store.last` -- last record from the store
	JsDeclareProperty(last);
	//#- `iter = store.forwardIter` -- returns iterator for iterating over the store from start to end
	JsDeclareProperty(forwardIter);
	//#- `iter = store.backwardIter` -- returns iterator for iterating over the store from end to start
	JsDeclareProperty(backwardIter);
	//#- `rec = store[recId]` -- get record with ID `recId`; 
	//#     returns `null` when no such record exists
	//JsDeclIndexedProperty(indexId); TODO
	//#- `rec = store.rec(recName)` -- get record named `recName`; 
	//#     returns `null` when no such record exists
	JsDeclareFunction(rec);
	//#- `store = store.each(callback)` -- iterates through the store and executes the callback function `callback` on each record. Same record JavaScript wrapper is used for all callback; to save record, make a clone (`rec.$clone()`). Returns self. Examples:
	//#  - `store.each(function (rec) { console.log(JSON.stringify(rec)); })`
	//#  - `store.each(function (rec, idx) { console.log(JSON.stringify(rec) + ', ' + idx); })`
	JsDeclareFunction(each);
	//#- `arr = store.map(callback)` -- iterates through the store, applies callback function `callback` to each record and returns new array with the callback outputs. Same record JavaScript wrapper is used for all callback; to save record, make a clone (`rec.$clone()`). Examples:
	//#  - `arr = store.map(function (rec) { return JSON.stringify(rec); })`
	//#  - `arr = store.map(function (rec, idx) {  return JSON.stringify(rec) + ', ' + idx; })`
	JsDeclareFunction(map);
	//#- `recId = store.add(rec)` -- add record `rec` to the store and return its ID `recId`
	JsDeclareFunction(add);
	//#- `rec = store.newRec(recordJson)` -- creates new record `rec` by (JSON) value `recordJson` (not added to the store)
	JsDeclareFunction(newRec);
	//#- `rs = store.newRecSet(idVec)` -- creates new record set from an integer vector record IDs `idVec` (type la.newIntVec);
	JsDeclareFunction(newRecSet);
	//#- `rs = store.sample(sampleSize)` -- create a record set containing a random 
	//#     sample of `sampleSize` records
	JsDeclareFunction(sample);
	//#- `field = store.field(fieldName)` -- get details of field named `fieldName`
	JsDeclareFunction(field);
	//#- `key = store.key(keyName)` -- get [index key](#index-key) named `keyName`
	JsDeclareFunction(key);
	//#- `store.addTrigger(trigger)` -- add `trigger` to the store triggers. Trigger is a JS object with three properties `onAdd`, `onUpdate`, `onDelete` whose values are callbacks
	JsDeclareFunction(addTrigger);
	//#- `sa = store.getStreamAggr(saName)` -- returns a stream aggregate `sa` whose name is `saName`
	JsDeclareFunction(getStreamAggr);
	//#- `strArr = store.getStreamAggrNames()` -- returns the names of all stream aggregators listening on the store as an array of strings `strArr`
	JsDeclareFunction(getStreamAggrNames);
	//#- `objJSON = store.toJSON()` -- returns the store as a JSON
	JsDeclareFunction(toJSON);
	//#- `store.clear()` -- deletes all records
	//#- `len = store.clear(num)` -- deletes the first `num` records and returns new length `len`
	JsDeclareFunction(clear);
	//#- `vec = store.getVec(fieldName)` -- gets the `fieldName` vector - the corresponding field type must be one-dimensional, e.g. float, int, string,...
	JsDeclareFunction(getVec);
	//#- `mat = store.getMat(fieldName)` -- gets the `fieldName` matrix - the corresponding field type must be float_v or num_sp_v
	JsDeclareFunction(getMat);
	//#- `val = store.cell(recId, fieldId)` -- if fieldId (int) corresponds to fieldName, this is equivalent to store[recId][fieldName]
	//#- `val = store.cell(recId, fieldName)` -- equivalent to store[recId][fieldName]
	JsDeclareFunction(cell);
	//#JSIMPLEMENT:src/qminer/store.js
public:
	TWPt<TQm::TBase> Base;
	TWPt<TQm::TStore> Store;
private:
	static v8::Persistent<v8::Function> constructor;
};


#endif

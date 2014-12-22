#ifndef QMINER_QM_NODEJS
#define QMINER_QM_NODEJS

#define BUILDING_NODE_EXTENSION

#include <node.h>
#include <node_object_wrap.h>
#include "qminer.h"
#include "../nodeutil.h"

///////////////////////////////
// NodeJs QMiner.
// A factory of base objects
// The only part of node framework: Init
class TNodeJsQm : public node::ObjectWrap {
public:
	// Node framework
	static void Init(v8::Handle<v8::Object> exports);
	static THash<TStr, TUInt> BaseFPathToId;
private:
	//# 
	//# **Functions and properties:**
	//# 
	//#- `qm.config(configPath, overwrite, portN, cahceSize)` -- create directory structure with basic qm.conf file. Optional parameters: `configPath` (='qm.conf'), `overwrite (= false)`, `portN` (=8080), `cacheSize` (=1024).
	JsDeclareFunction(config);
	//#- `base = qm.create(configPath, schemaPath)` -- creates an empty base using the configuration in `configPath` and schema described in `schemaPath` (optional)
	JsDeclareFunction(create);
	//#- `base = qm.open(configPath, readOnly)` -- opens a base using the configuration in `configPath` using `readOnly` (boolean) parameter
	JsDeclareFunction(open);	

	JsDeclareFunction(test);
};

///////////////////////////////
// NodeJs QMiner Base
class TNodeJsBase : public node::ObjectWrap {
private:
	// Node framework
	static v8::Persistent<v8::Function> constructor;
public:
	// Node framework
	static void Init(v8::Handle<v8::Object> exports);
	// Wrapping C++ object
	static v8::Local<v8::Object> New(TQm::PBase _Base);
    // C++ constructors
	TNodeJsBase() { }
	TNodeJsBase(TQm::PBase _Base) : Base(_Base) { }
	// Node framework (constructor method)
	JsDeclareFunction(New);
    
public:
	// C++ wrapped object
	TQm::PBase Base;

private:
	//# 
	//# **Functions and properties:**
	//# 
	//#- `base.close()` -- closes the base
	JsDeclareFunction(close);
    //#- `store = base.store(storeName)` -- return store with name `storeName`; `store = null` when no such store
	JsDeclareFunction(store);
    //#- `strArr = base.getStoreList()` -- an array of strings listing by name all existing stores
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
};


///////////////////////////////
// NodeJs-Qminer-Store
class TNodeJsStore : public node::ObjectWrap {
private:
	// Node framework
	static v8::Persistent<v8::Function> constructor;
public:
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
	//static v8::Local<v8::Object> Field(const TWPt<TQm::TStore>& Store, const TQm::TRec& Rec, const int FieldId);
	static v8::Local<v8::Value> Field(const TWPt<TQm::TStore>& Store, const uint64& RecId, const int FieldId);

public:
	// C++ wrapped object
	TWPt<TQm::TStore> Store;

private:
	//# 
	//# **Functions and properties:**
	//#
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
	JsDeclIndexedProperty(indexId);	
	//#JSIMPLEMENT:src/qminer/store.js
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
	//#
	//# **Functions and properties:**
	//#
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
	//# 
	//# **Functions and properties:**
	//#   
	//#- `rs2 = rs.clone()` -- creates new instance of record set
	JsDeclareFunction(clone);
	//#- `rs2 = rs.join(joinName)` -- executes a join `joinName` on the records in the set, result is another record set `rs2`.
	//#- `rs2 = rs.join(joinName, sampleSize)` -- executes a join `joinName` on a sample of `sampleSize` records in the set, result is another record set `rs2`.
	JsDeclareFunction(join);
	//#- `aggrsJSON = rs.aggr()` -- returns an object where keys are aggregate names and values are JSON serialized aggregate values of all the aggregates contained in the records set
	//#- `aggr = rs.aggr(aggrQueryJSON)` -- computes the aggregates based on the `aggrQueryJSON` parameter JSON object. If only one aggregate is involved and an array of JSON objects when more than one are returned.
	JsDeclareFunction(aggr);
	//#- `rs = rs.trunc(limit_num)` -- truncate to first `limit_num` record and return self.
	//#- `rs = rs.trunc(limit_num, offset_num)` -- truncate to `limit_num` record starting with `offset_num` and return self.
	JsDeclareFunction(trunc);
	//#- `rs2 = rs.sample(num)` -- create new record set by randomly sampling `num` records.
	JsDeclareFunction(sample);
	//#- `rs = rs.shuffle(seed)` -- shuffle order using random integer seed `seed`. Returns self.
	JsDeclareFunction(shuffle);
	//#- `rs = rs.reverse()` -- reverse record order. Returns self.
	JsDeclareFunction(reverse);
	//#- `rs = rs.sortById(asc)` -- sort records according to record id; if `asc > 0` sorted in ascending order. Returns self.
	JsDeclareFunction(sortById);
	//#- `rs = rs.sortByFq(asc)` -- sort records according to weight; if `asc > 0` sorted in ascending order. Returns self.
	JsDeclareFunction(sortByFq);
	//#- `rs = rs.sortByField(fieldName, asc)` -- sort records according to value of field `fieldName`; if `asc > 0` sorted in ascending order. Returns self.
	JsDeclareFunction(sortByField);
	//#- `rs = rs.sort(comparatorCallback)` -- sort records according to `comparator` callback. Example: rs.sort(function(rec,rec2) {return rec.Val < rec2.Val;} ) sorts rs in ascending order (field Val is assumed to be a num). Returns self.
	JsDeclareFunction(sort);
	//#- `rs = rs.filterById(minId, maxId)` -- keeps only records with ids between `minId` and `maxId`. Returns self.
	JsDeclareFunction(filterById);
	//#- `rs = rs.filterByFq(minFq, maxFq)` -- keeps only records with weight between `minFq` and `maxFq`. Returns self.
	JsDeclareFunction(filterByFq);
	//#- `rs = rs.filterByField(fieldName, minVal, maxVal)` -- keeps only records with numeric value of field `fieldName` between `minVal` and `maxVal`. Returns self.
	//#- `rs = rs.filterByField(fieldName, minTm, maxTm)` -- keeps only records with value of time field `fieldName` between `minVal` and `maxVal`. Returns self.
	//#- `rs = rs.filterByField(fieldName, str)` -- keeps only records with string value of field `fieldName` equal to `str`. Returns self.
	JsDeclareFunction(filterByField);
	//#- `rs = rs.filter(filterCallback)` -- keeps only records that pass `filterCallback` function. Returns self.
	JsDeclareFunction(filter);
	//#- `rsArr = rs.split(splitterCallback)` -- split records according to `splitter` callback. Example: rs.split(function(rec,rec2) {return (rec2.Val - rec2.Val) > 10;} ) splits rs in whenever the value of field Val increases for more than 10. Result is an array of record sets. 
	JsDeclareFunction(split);
	//#- `rs = rs.deleteRecs(rs2)` -- delete from `rs` records that are also in `rs2`. Returns self.
	JsDeclareFunction(deleteRecs);
	//#- `objsJSON = rs.toJSON()` -- provide json version of record set, useful when calling JSON.stringify
	JsDeclareFunction(toJSON);
	//#- `rs = rs.each(callback)` -- iterates through the record set and executes the callback function `callback` on each element. Same record JavaScript wrapper is used for all callback; to save record, make a clone (`rec.$clone()`). Returns self. Examples:
	//#  - `rs.each(function (rec) { console.log(JSON.stringify(rec)); })`
	//#  - `rs.each(function (rec, idx) { console.log(JSON.stringify(rec) + ', ' + idx); })`
	JsDeclareFunction(each);
	//#- `arr = rs.map(callback)` -- iterates through the record set, applies callback function `callback` to each element and returns new array with the callback outputs. Same record JavaScript wrapper is used for all callback; to save record, make a clone (`rec.$clone()`). Examples:
	//#  - `arr = rs.map(function (rec) { return JSON.stringify(rec); })`
	//#  - `arr = rs.map(function (rec, idx) {  return JSON.stringify(rec) + ', ' + idx; })`
	JsDeclareFunction(map);
	//#- `rs3 = rs.setintersect(rs2)` -- returns the intersection (record set) `rs3` between two record sets `rs` and `rs2`, which should point to the same store.
	JsDeclareFunction(setintersect);
	//#- `rs3 = rs.setunion(rs2)` -- returns the union (record set) `rs3` between two record sets `rs` and `rs2`, which should point to the same store.
	JsDeclareFunction(setunion);
	//#- `rs3 = rs.setdiff(rs2)` -- returns the set difference (record set) `rs3`=`rs`\`rs2`  between two record sets `rs` and `rs1`, which should point to the same store.
	JsDeclareFunction(setdiff);
	//#- `vec = rs.getVec(fieldName)` -- gets the `fieldName` vector - the corresponding field type must be one-dimensional, e.g. float, int, string,...
	JsDeclareFunction(getVec);
	//#- `vec = rs.getMat(fieldName)` -- gets the `fieldName` matrix - the corresponding field type must be float_v or num_sp_v
	JsDeclareFunction(getMat);
	
	//#- `storeName = rs.store` -- store of the records
	JsDeclareProperty(store);
	//#- `len = rs.length` -- number of records in the set
	JsDeclareProperty(length);
	//#- `bool = rs.empty` -- `bool = true` when record set is empty
	JsDeclareProperty(empty);
	//#- `bool =  rs.weighted` -- `bool = true` when records in the set are assigned weights
	JsDeclareProperty(weighted);
	//#- `rec = rs[n]` -- return n-th record from the record set
	JsDeclIndexedProperty(indexId);
};

///////////////////////////////
// NodeJs QMiner Store Iterator
//# 
//# ### Store iterator
//# 
class TNodeJsStoreIter: public node::ObjectWrap {
private:
	// Node framework
	static v8::Persistent<v8::Function> constructor;
public:
	// Node framework 
	static void Init(v8::Handle<v8::Object> exports);
	// Wrapping C++ object	
	static v8::Local<v8::Object> New();
	static v8::Local<v8::Object> New(const TWPt<TQm::TStore>& Store, const TWPt<TQm::TStoreIter>& Iter);
	// C++ constructors
	TNodeJsStoreIter() {}
	TNodeJsStoreIter(const TWPt<TQm::TStore>& _Store, const TWPt<TQm::TStoreIter>& _Iter) : Store(_Store), Iter(_Iter) {}
	// Node framework (constructor method)
	JsDeclareFunction(New);
	
public:
	// C++ wrapped object
	TWPt<TQm::TStore> Store;	
	TWPt<TQm::TStoreIter> Iter;
    // placeholder for last object
	v8::Persistent<v8::Object> RecObj;
	TNodeJsRec* JsRec;
    // delete placeholder
    ~TNodeJsStoreIter() { RecObj.Reset(); }
	
    //#
	//# **Functions and properties:**
	//#   
	//#- `bool = iter.next()` -- moves to the next record or returns false if no record left; must be called at least once before `iter.rec` is available
	JsDeclareFunction(next);
	//#- `store = iter.store` -- get the store
	JsDeclareProperty(store);
	//#- `rec = iter.rec` -- get current record; reuses JavaScript record wrapper, need to call `rec.$clone()` on it to if there is any wish to store intermediate records.
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
//#
//# ### Index key
//#
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
    //#
	//# **Functions and properties:**
	//#   
	//#- `store = key.store` -- gets the key's store
	JsDeclareProperty(store);
	//#- `keyName = key.name` -- gets the key's name
	JsDeclareProperty(name);
	//#- `strArr = key.voc` -- gets the array of words (as strings) in the vocabulary
	JsDeclareProperty(voc);
	//#- `strArr = key.fq` -- gets the array of weights (as strings) in the vocabulary
	JsDeclareProperty(fq);
};



#endif

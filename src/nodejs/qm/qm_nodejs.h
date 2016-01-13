/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef QMINER_QM_NODEJS
#define QMINER_QM_NODEJS

#include <node.h>
#include <node_object_wrap.h>
#include <qminer.h>
#include "../la/la_nodejs.h"
#include "../fs/fs_nodejs.h"
#include "../nodeutil.h"

#include "qm_nodejs_streamaggr.h"
#include "qm_nodejs_store.h"

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
	// TNodeJsRec needs this to select a template. TODO remove, see comment in
    //   v8::Local<v8::Object> TNodeJsRec::New(const TQm::TRec& Rec, const TInt& _Fq)
	static THash<TStr, TUInt> BaseFPathToId;
private:
	/*
	* Creates a directory structure.
	* @param {string} [configPath='qm.conf'] - The path to configuration file.
	* @param {boolean} [overwrite=false] - If you want to overwrite the configuration file.
	* @param {number} [portN=8080] - The number of the port. Currently not used.
	* @param {number} [cacheSize=1024] - Sets available memory for indexing (in MB).
	*/
	// exports.config = function(configPath, overwrite, portN, cacheSize) {}
	JsDeclareFunction(config);

	/*
	* Creates an empty base.
	* @param {string} [configPath='qm.conf'] - Configuration file path.
	* @param {string} [schemaPath=''] - Schema file path.
	* @param {boolean} [clear=false] - Clear the existing db folder.
	* @returns {module:qm.Base}
	*/
	// exports.create = function (configPath, schemaPath, clear) { return Object.create(require('qminer').Base.prototype); }
	JsDeclareFunction(create);

	/*
	* Opens a base.
	* @param {string} [configPath='qm.conf'] - The configuration file path.
	* @param {boolean} [readOnly=false] - Open in read only mode?
	* @returns {module:qm.Base}
	*/
	// exports.open = function (configPath, readOnly) { return Object.create(require('qminer').Base.prototype); }
	JsDeclareFunction(open);
    
	/**
	* Set verbosity of QMiner internals.
	* @param {number} [level=0] - verbosity level: 0 = no output, 1 = log output, 2 = log and debug output.
	*/
	//# exports.verbosity = function (level) { }
	JsDeclareFunction(verbosity);
    
   	/**
	* @property {Object} flags - Returns an object with all compile flags
	*/
	//# exports.flags = {};
	JsDeclareProperty(flags);
};

///////////////////////////////
// NodeJs QMiner Base

/**
* Base access modes.
* @readonly
* @enum {string}
*/
//# var BaseModes = {
//#    /** sets up the db folder */
//#    create: 'create',
//#    /** cleans the db folder and calls create */
//#    createClean: 'createClean',
//#    /** opens with write permissions */
//#    open: 'open',
//#    /** opens in read-only mode */
//#    openReadOnly: 'openReadOnly'
//# }

/**
* Base constructor parameter object
* @typedef {Object} BaseConstructorParam
* @property  {module:qm~BaseModes} [BaseConstructorParam.mode='openReadOnly'] - Base access mode: 
* <br> create (sets up the db folder), 
* <br> createClean (cleans db folder and then sets it up), 
* <br> open (opens the db with read/write permissions), 
* <br> openReadOnly (opens the db in read only mode).
* @property  {number} [BaseConstructorParam.indexCache=1024] - The ammount of memory reserved for indexing (in MB).
* @property  {number} [BaseConstructorParam.storeCache=1024] - The ammount of memory reserved for store cache (in MB).
* @property  {string} [BaseConstructorParam.schemaPath=''] - The path to schema definition file.
* @property  {Array<module:qm~SchemaDefinition>} [BaseConstructorParam.schema=[]] - Schema definition object array.
* @property  {string} [BaseConstructorParam.dbPath='./db/'] - The path to db directory.
*/

/**
* Store schema definition object
* @typedef {Object} SchemaDefinition
* @property {string} SchemaDefinition.name - The name of the store. Store name can be composed by from English letters, numbers, _ or $ characters. It can only begin with a character.
* @property {Array<module:qm~SchemaFieldDefinition>} SchemaDefinition.fields - The array of field descriptors. 
* @property {Array<module:qm~SchemaJoinDefinition>} [SchemaDefinition.joins=[]] - The array of join descriptors, used for linking records from different stores.
* @property {Array<module:qm~SchemaKeyDefinition>} [SchemaDefinition.keys=[]] - The array of key descriptors. Keys define how records are indexed, which is needed for search using the query language.
* @property {module:qm~SchemaTimeWindowDefinition} [SchemaDefinition.timeWindow] - Time window description. Stores can have a window, which is used by garbage collector to delete records once they fall out of the time window. Window can be defined by number of records or by time.
* @example
* var qm = require('qminer');
* // create a simple movies store, where each record contains only the movie title.
* var base = new qm.Base({
*     mode: 'createClean',
*     schema: [{
*       name: "Movies",
*       fields: [{ name: "title", type: "string" }]
*     }]
* });
* base.close();
*/

/**
* Field types.
* @readonly
* @enum {string}
*/
//# var FieldTypes = {
//#		/** signed 32-bit integer */
//#		int: 'int', 
//#		/** vector of signed 32-bit integers */
//#		int_v: 'int_v', 
//#		/** string */
//#		string : 'string',
//#		/** vector of strings */
//#		string_v : 'string_v',
//#		/** boolean */
//#		bool : 'bool',
//#		/** double precision floating point number */
//#		float : 'float',
//#		/** a pair of floats, useful for storing geo coordinates */
//#		float_pair : 'float_pair',
//#		/** vector of floats */
//#		float_v : 'float_v',
//#		/** date and time format, stored in a form of milliseconds since 1600 */
//#		datetime : 'datetime',
//#		/** sparse vector(same format as used by QMiner JavaScript linear algebra library) */
//#		num_sp_v : 'num_sp_v',
//# }

/**
* Store schema field definition object
* @typedef {Object} SchemaFieldDefinition
* @property {string} SchemaFieldDefinition.name - The name of the field.
* @property {module:qm~FieldTypes} SchemaFieldDefinition.type - The type of the field.
* @property {boolean} [SchemaFieldDefinition.primary=false] - Field which can be used to identify record. There can be only one primary field in a store. There can be at most one record for each value of the primary field. Currently following fields can be marked as primary: int, uin64, string, float, datetime. Primary fields of type string are also used for record names.
* @property {boolean} [SchemaFieldDefinition.null=false] - When set to true, null is a possible value for a field (allow missing values).
* @property {string} [SchemaFieldDefinition.store='memory'] - Defines where to store the field, options are: <b>'cache'</b> or <b>'memory'</b>. The default option is <b>'memory'</b>, which stores the values in RAM. Option <b>'cache'</b> stores the values on disk, with a layer of FIFO cache in RAM, storing the most recently used values.
* @property {Object} [SchemaFieldDefinition.default] - Default value for field when not given for a new record.
* @property {boolean} [SchemaFieldDefinition.codebook=false] - Useful when many records have only few different values of this field. If set to true, then a separate table of all values is kept, and records only point to this table (replacing variable string field in record serialisation with fixed-length integer). Useful to decrease memory footprint, and faster to update. (STRING FIELD TYPE SPECIFIC).
* @property {boolean} [SchemaFieldDefinition.shortstring=false] - Useful for string shorter then 127 characters (STRING FIELD TYPE SPECIFIC).
* @example
*  var qm = require('qminer');
*  var base = new qm.Base({
*      mode: 'createClean',
*      schema: [
*        { name: 'NewsArticles',
*          fields: [
*            { name: "ID", primary: true, type: "string", shortstring: true },
*            { name: "Source", type: "string", codebook: true },
*            { name: "DateTime", type: "datetime" },
*            { name: "Title", type: "string", store: "cache" },
*            { name: "Tokens", type: "string_v", store: "cache", null: true },
*            { name: "Vector", type: "num_sp_v", store: "cache", null: true }]
*        }
*     ]
*  });
* // add a record:
* // - we set the date using the ISO string representation
* // - we set the string vector Tokens with an array of strings
* // - we set the numeric sparse vector Vector with an array of two element arrays
* //   (index, value), see the sparse vector constructor {@link module:la.SparseVector}
* base.store('NewsArticles').push({
*   ID: 't12344', 
*   Source: 's1234', 
*   DateTime: '2015-01-01T00:05:00', 
*   Title: 'the title', 
*   Tokens: ['token1', 'token2'], 
*   Vector: [[0,1], [1,1]]});
* base.close();
*/

/**
* Store schema join definition object
* @typedef {Object} SchemaJoinDefinition
* @property {string} SchemaJoinDefinition.name - The name of the join.
* @property {string} SchemaJoinDefinition.type - The supported types are: <b>'field'</b> and <b>'index'</b>. 
* <br> A join with type=<b>'field'</b> can point to zero or one record and is implemented as an additional hidden field of type uint64, which can hold the ID of the record it links to. Accessing the record's join returns a record.
* <br> A join with type=<b>'index'</b> can point to any number of records and is implemented using the inverted index, where for each record a list (vector) of linked records is kept. Accessing the record's join returns a record set.
* @property {string} SchemaJoinDefinition.store - The store name from which the linked records are.
* @example
* var qm = require('qminer');
* // Create two stores: People which stores only names of persons and Movies, which stores only titles.
* // Each person can direct zero or more movies, so we use an index join named 'directed' and
* // each movie has a single director, so we use a field join 'director'. The joins are 
* // inverses of each other. The inverse join simplifies the linking, since only one join needs
* // to be specified, and the other direction can be linked automatically (in the example 
* // below we specify only the 'director' link and the 'directed' join is updated automatically).
* //
* var base = new qm.Base({
*     mode: 'createClean',
*     schema: [
*       { name: 'People', 
*         fields: [{ name: 'name', type: 'string', primary: true }], 
*         joins: [{ name: 'directed', 'type': 'index', 'store': 'Movies', 'inverse': 'director' }] },
*       { name: 'Movies', 
*         fields: [{ name: 'title', type: 'string', primary: true }], 
*         joins: [{ name: 'director', 'type': 'field', 'store': 'People', 'inverse': 'directed' }] }
*     ]
* });
* // Adds a movie, automatically adds 'Jim Jarmusch' to People, sets the 'director' join (field join)
* // and automatically updates the index join 'directed', since it's an inverse join of 'director'
* base.store('Movies').push({ title: 'Broken Flowers', director: { name: 'Jim Jarmusch' } });
* // Adds a movie, sets the 'director' join, updates the index join of 'Jim Jarmusch'
* base.store('Movies').push({ title: 'Coffee and Cigarettes', director: { name: 'Jim Jarmusch' } });
* // Adds movie, automatically adds 'Lars von Trier' to People, sets the 'director' join
* // and 'directed' inverse join (automatically)
* base.store('Movies').push({ title: 'Dogville', director: { name: 'Lars von Trier' } });
*
* var movie = base.store('Movies')[0]; // get the first movie (Broken Flowers)
* // Each movie has a property corresponding to the join name: 'director'. 
* // Accessing the property returns a {@link module:qm.Record} from the store People.
* var person = movie.director; // get the director
* var personName = person.name; // get person's name ('Jim Jarmusch')
* // Each person has a property corresponding to the join name: 'directed'. 
* // Accessing the property returns a {@link module:qm.RecSet} from the store People.
* var movies = person.directed; // get all the movies the person directed.
* movies.each(function (movie) { var title = movie.title; });
* // Gets the following titles:
* //   'Broken Flowers'
* //   'Coffee and Cigarettes'
* base.close();
*/

/**
* Store schema key definition object
* @typedef {Object} SchemaKeyDefinition
* @property {string} SchemaKeyDefinition.field - The name of the field that will be indexed.
* @property {string} SchemaKeyDefinition.type - The supported types are: <b>'value'</b>, <b>'text'</b> and <b>'location'</b>.
* <br> A key with type=<b>'value'</b> indexes records using an inverted index using full value of the field (no processing).
*  The key type supports 'string', 'string_v' and 'datetime' fields types.
* <br> A key with type=<b>'text'</b> indexes string fields by using a tokenizer and text processing. Supported by string fields.
* <br> A key with type=<b>'location'</b> indexes records as points on a sphere and enables nearest-neighbour queries. Supported by float_pair type fields.
* @property {string} [SchemaKeyDefinition.name] - Allows using a different name for the key in search queries. This allows for multiple keys to be put against the same field. Default value is the name of the field.
* @property {string} [SchemaKeyDefinition.vocabulary] - defines the name of the vocabulary used to store the tokens or values. This can be used indicate to several keys to use the same vocabulary, to save on memory. Supported by 'value' and 'text' keys.
* @property {string} [SchemaKeyDefinition.tokenize] - defines the tokenizer that is used for tokenizing the values stored in indexed fields. Tokenizer uses same parameters as in bag-of-words feature extractor. Default is english stopword list and no stemmer. Supported by 'text' keys.
* @example
* var qm = require('qminer');
* // Create a store People which stores only names of persons.
* var base = new qm.Base({
*     mode: 'createClean',
*     schema: [
*         { name: 'People',
*           fields: [{ name: 'name', type: 'string', primary: true }],
*           keys: [
*             { field: 'name', type: 'value'}, 
*             { field: 'name', name: 'nameText', type: 'text'}
*          ]
*        }
*     ]
* });
*
* base.store('People').push({name : 'John Smith'});
* base.store('People').push({name : 'Mary Smith'});
* // search based on indexed values
* base.search({$from : 'People', name: 'John Smith'}); // Return the record set containing 'John Smith'
* // search based on indexed values
* base.search({$from : 'People', name: 'Smith'}); // Returns the empty record set.
* // search based on text indexing
* base.search({$from : 'People', nameText: 'Smith'}); // Returns both records.
* base.close();
*/

/**
* Stores can have a window, which is used by garbage collector to delete records once they
* fall out of the time window. Window can be defined by number of records or by time.
* Window defined by parameter window, its value being the number of records to be kept.
* <br><b>Important:</b> {@link module:qm.Base#garbageCollect} must be called manually to remove records outside time window.
* @typedef {Object} SchemaTimeWindowDefinition
* @property {number} SchemaTimeWindowDefinition.duration - the size of the time window (in number of units).
* @property {string} SchemaTimeWindowDefinition.unit - defines in which units the window size is specified. Possible values are <b>second</b>, <b>minute</b>, <b>hour</b>, <b>day</b>, <b>week</b> or <b>month</b>.
* @property {string} [SchemaTimeWindowDefinition.field] - name of the datetime filed, which defines the time of the record. In case it is not given, the insert time is used in its place.
* @example <caption>Define window by number of records</caption>
* var qm = require('qminer');
* // create base
* var base = new qm.Base({ mode: 'createClean' });
* // create store with window
* base.createStore({
*     "name": "TestStore",
*     "fields": [
*         { "name": "DateTime", "type": "datetime" },
*         { "name": "Measurement", "type": "float" }
*     ],
*     window: 3,
* });
*
* // push 5 records into created store
* for (var i = 0; i < 5; i++) {
*     var rec = {
*         "DateTime": new Date().toISOString(),
*         "Measurement": i
*     };
*     base.store("TestStore").push(rec);
* }
*
* // check number of records in store
* console.log(base.store("TestStore").allRecords.length); // 5
* // clean base with garbage collector
* base.garbageCollect();
* // check number of records in store
* console.log(base.store("TestStore").allRecords.length); // 3
*
* base.close();
* @example <caption>Define window by time</caption>
* var qm = require('qminer');
* // create base
* var base = new qm.Base({ mode: 'createClean' });
* // create store with window
* base.createStore({
*     "name": "TestStore",
*     "fields": [
*         { "name": "DateTime", "type": "datetime" },
*         { "name": "Measurement", "type": "float" }
*     ],
*     timeWindow: {
*         duration: 2,
*         unit: "hour",
*         field: "DateTime"
*     }
* });
*
* // push 5 records into created store
* for (var i = 0; i < 5; i++) {
*     var rec = {
*         "DateTime": new Date(new Date().getTime() + i * 60 * 60 * 1001).toISOString(),
*         "Measurement": i
*     };
*     base.store("TestStore").push(rec);
* }
*
* // check number of records in store
* console.log(base.store("TestStore").allRecords.length); // 5
* // clean base with garbage collector
* base.garbageCollect();
* // check number of records in store
* console.log(base.store("TestStore").allRecords.length); // 2
*
* base.close();
*/


class TNodeJsBaseWatcher {
private:
	// smart pointer
	TCRef CRef;
	friend class TPt<TNodeJsBaseWatcher>;
public:
	bool OpenP;
	TNodeJsBaseWatcher() { OpenP = true; }
	static TPt<TNodeJsBaseWatcher> New() { return new TNodeJsBaseWatcher; }
	void AssertOpen() { EAssertR(OpenP, "Base is closed!"); }
	void Close() { OpenP = false; }
	bool IsClosed() const { return !OpenP; }
};
typedef TPt<TNodeJsBaseWatcher> PNodeJsBaseWatcher;

/**
* Base
* @classdesc Represents the database and holds stores. The base object can be opened in multiple
* modes: 'create' - create a new database, 'createClean' - force create, and 'openReadOnly' - open in read-only mode.
* @class
* @param {module:qm~BaseConstructorParam} paramObj - The base constructor parameter object.
* @property {String} paramObj.mode - The mode in which base is opened.
* @property [String] paramObj.dbPath - The path to the location of the database.
* @property [Object] paramObj.schema - The database schema.
* @example
* // import qm module
* var qm = require('qminer');
* // using a constructor, in open mode
* var base = new qm.Base({mode: 'open'});
* base.close();
*/
//# exports.Base = function (paramObj) { return Object.create(require('qminer').Base.prototype); };
class TNodeJsBase : public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	static v8::Persistent<v8::Function> Constructor;
public:
	static void Init(v8::Handle<v8::Object> Exports);
	static const TStr GetClassId() { return "Base"; }
	// wrapped C++ object
	TWPt<TQm::TBase> Base;
	// C++ constructor
	TNodeJsBase(const TWPt<TQm::TBase>& Base_) : Base(Base_) { Watcher = TNodeJsBaseWatcher::New(); }
	TNodeJsBase(const TStr& DbPath, const TStr& SchemaFNm, const PJsonVal& Schema,
        const bool& Create, const bool& ForceCreate, const bool& ReadOnly,
        const uint64& IndexCache, const uint64& StoreCache);
	// Object that knows if Base is valid
	PNodeJsBaseWatcher Watcher;
private:		
	// parses arguments, called by javascript constructor 
	static TNodeJsBase* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);
private:
	/**
	* Closes the database.
	* @returns {null}
	*/
	//# exports.Base.prototype.close = function () { return null; }
	JsDeclareFunction(close);

	/**
	* Checks if the base is closed.
	* @returns {Boolean}
	*/
	JsDeclareFunction(isClosed);

	/**
	 * Returns the store with the specified name.
	 * @param {string} name - Name of the store.
	 * @returns {module:qm.Store} The store.
	 * @example
	 * // import qm module
	 * var qm = require('qminer');
	 * // create a base with two stores
	 * var base = new qm.Base({
	 *    mode: "createClean",
	 *    schema: [
	 *    {
	 *        name: "KwikEMart",
	 *        fields: [
	 *            { name: "Worker", type: "string" },
	 *            { name: "Groceries", type: "string_v" }
	 *        ]
	 *    },
	 *    {
	 *        name: "NuclearPowerplant",
	 *        fields: [
	 *            { name: "Owner", type: "string" },
	 *            { name: "NumberOfAccidents", type: "int" },
	 *            { name: "Workers", type: "string_v" }
	 *        ]
	 *    }]
	 * });
	 * // get the "KwikEMart" store 
	 * var store = base.store("KwikEMart");	// returns the store with the name "KwikEMart"
	 * base.close();
	 */
	//# exports.Base.prototype.store = function (name) { return Object.create(require('qminer').Store.prototype); }
	JsDeclareFunction(store);

	/**
	 * Returns a list of store descriptors.
	 * @returns {Object[]} The list of store descriptors.
	 */
	//# exports.Base.prototype.getStoreList = function () { return [{storeId:'', storeName:'', storeRecords:'', fields: [], keys: [], joins: []}]; }
	JsDeclareFunction(getStoreList);

	/**
	* Creates a new store.
	* @param {Array.<module:qm~SchemaDefinition>} storeDef - The definition of the store(s).
	* @param {number} [storeSizeInMB = 1024] - The reserved size of the store(s).
	* @returns {(module:qm.Store | module:qm.Store[])} - Returns a store or an array of stores (if the schema definition was an array).
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base with one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [
	*    {
	*        name: "Superheroes",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Superpowers", type: "string_v" },
	*            { name: "YearsActive", type: "int" }
	*        ]
	*    }]
	* });
	* // create a new store called "Supervillains" in the base
	* base.createStore({
	*    name: "Supervillians",
	*    fields: [
	*        { name: "Name", type: "string" },
	*        { name: "Superpowers", type: "string_v" },
	*        { name: "YearsActive", type: "int" }
	*    ]
	* });
	* // create two new stores called "Cities" and "Leagues"
	* base.createStore([
	*    {
	*        name: "Cities",
	*        fields: [
	*            { name: "Name", type: "string", primary: true },
	*            { name: "Population", type: "int" }
	*        ]
	*    },
	*    {
	*        name: "Leagues",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Members", type: "string_v" }
	*        ]
	*    }
	* ]);
	* base.close();
	*/
	//# exports.Base.prototype.createStore = function (storeDef, storeSizeInMB) { return storeDef instanceof Array ? [Object.create(require('qminer').Store.prototype)] : Object.create(require('qminer').Store.prototype) ;}
	JsDeclareFunction(createStore);
	
	// Creates a javascript implemented stores (callbacks). Experimental feature!
	JsDeclareFunction(createJsStore);

	// adds a callback to a javascript implemented store (type == 'TNodeJsFuncStore')
	// arg[0] = store, arg[1] = callback string, arg[2] = callback function
	// Experimental feature!
	JsDeclareFunction(addJsStoreCallback);

	/**
	* Creates a new store.
	* @param {module:qm~QueryObject} query - query language JSON object	
	* @returns {module:qm.RecordSet} - Returns the record set that matches the search criterion
	*/
	//# exports.Base.prototype.search = function (query) { return Object.create(require('qminer').RecordSet.prototype); }

	JsDeclareFunction(search);   

	/**
	* Calls qminer garbage collector to remove records outside time windows.
	*/
	//# exports.Base.prototype.garbageCollect = function () { }
	JsDeclareFunction(garbageCollect);

	/**
	* Calls qminer partial flush - base saves dirty data given some time-window.
	* @param {number} window - Length of available time-window in msec. Default 500.
	*/
	//# exports.Base.prototype.partialFlush = function () { }

	JsDeclareFunction(partialFlush);

	/**
	* Retrieves performance statistics for qminer.
	*/
	//# exports.Base.prototype.getStats = function () { }
	JsDeclareFunction(getStats);

	/**
	* Gets the stream aggregate of the given name.
	* @param {string} saName - The name of the stream aggregate.
	* @returns {module:qm.StreamAggr} The stream aggregate whose name is saName.
	*/
	//# exports.Base.prototype.getStreamAggr = function (saName) { return Object.create(require('qminer').StreamAggr.prototype); }
	JsDeclareFunction(getStreamAggr);

	/**
	* Gets an array of the stream aggregate names in the base.
	* @returns {Array.<string>} The array containing the stream aggregat names.
	*/
	//# exports.Base.prototype.getStreamAggrNames = function () { return [""]; }
	JsDeclareFunction(getStreamAggrNames);	
	//!JSIMPLEMENT:src/qminer/qminer.js    
};


///////////////////////////////
// NodeJs-Qminer-Store

/**
* Stores are containers of records. 
* <b>Factory pattern:</b> this class cannot be construced using the new keyword. This class is constructed when 
* calling a specific method or attribute, e.g. constructing the {@link module:qm.Base} using schema or with the 
* {@link module:qm.Base#createStore}.
* @class
* @example <caption>Creating a store with createStore function</caption>
* // import qm module
* var qm = require('qminer');
* // factory based construction using base.createStore
* var base = new qm.Base({ mode: 'createClean' });
* base.createStore([{
*    name: "People",
*    fields: [
*        { name: "Name", type: "string", primary: true },
*        { name: "Gender", type: "string", shortstring: true },
*        { name: "Age", type: "int" }
*    ],
*    joins: [
*        { name: "ActedIn", type: "index", store: "Movies", inverse: "Actor" },
*        { name: "Directed", type: "index", store: "Movies", inverse: "Director" }
*    ],
*    keys: [
*        { field: "Name", type: "text" },
*        { field: "Gender", type: "value" }
*    ]
* },
* {
*    name: "Movies",
*    fields: [
*        { name: "Title", type: "string", primary: true },
*        { name: "Plot", type: "string", store: "cache" },
*        { name: "Year", type: "int" },
*        { name: "Rating", type: "float" },
*        { name: "Genres", type: "string_v", codebook: true }
*    ],
*    joins: [
*        { name: "Actor", type: "index", store: "People", inverse: "ActedIn" },
*        { name: "Director", type: "index", store: "People", inverse: "Directed" }
*    ],
*    keys: [
*        { field: "Title", type: "value" },
*        { field: "Plot", type: "text", vocabulary: "voc_01" },
*        { field: "Genres", type: "value" }
*    ]
* }]);
* base.close();
* @example <caption>Creating store with schema in base constructor</caption>
* // import qm module
* var qm = require('qminer');
* // using the base constructor
* var base = new qm.Base({
*    mode: "createClean",
*    schema: [{
*        name: "Class",
*        fields: [
*            { name: "Name", type: "string" },
*            { name: "StudyGroup", type: "string" }
*        ]
*    }]
* });
* base.close();
*/
//# exports.Store = function (base, storeDef) { return Object.create(require('qminer').Store.prototype); };
class TNodeJsStore : public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	// Node framework
	static v8::Persistent<v8::Function> Constructor;
public:	
	// Node framework 
	static void Init(v8::Handle<v8::Object> exports);
	static const TStr GetClassId() { return "Store"; }
	// Wrapped C++ object
	TWPt<TQm::TStore> Store;
	// Object that knows if Base is valid
	PNodeJsBaseWatcher Watcher;
	// C++ constructors	
	TNodeJsStore(TWPt<TQm::TStore> _Store, PNodeJsBaseWatcher& _Watcher) : Store(_Store), Watcher(_Watcher) { }

	// Field accessors
	static v8::Local<v8::Value> Field(const TQm::TRec& Rec, const int FieldId);
	static v8::Local<v8::Value> Field(const TWPt<TQm::TStore>& Store, const uint64& RecId, const int FieldId);
private:
	
	/**
	* Returns a record from the store.
	* @param {string} recName - Record name.
	* @returns {(module:qm.Record | null)} Returns the record. If the record doesn't exist, it returns null.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a base containing the store Class. Let the Name field be the primary field. 
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Class",
	*        fields: [
	*            { name: "Name", type: "string", primary: true },
	*            { name: "StudyGroup", type: "string" }
	*        ]
	*    }]
	* });
	* // add some records to the store
	* base.store("Class").push({ Name: "Dean", StudyGroup: "A" });
	* base.store("Class").push({ Name: "Chang", StudyGroup: "D" });
	* base.store("Class").push({ Name: "Magnitude", StudyGroup: "C" });
	* base.store("Class").push({ Name: "Leonard", StudyGroup: "B" });
	* // get the record with the name "Magnitude"
	* var record = base.store("Class").recordByName("Magnitude");
	* base.close();
	*/
	//# exports.Store.prototype.recordByName = function (recName) { return Object.create(require('qminer').Record.prototype); };
	JsDeclareFunction(recordByName);

	/**
	* Executes a function on each record in store.
	* @param {function} callback - Function to be executed. It takes two parameters:
	* <br>rec - The current record.
	* <br>[idx] - The index of the current record.
	* @returns {module:qm.Store} Self.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a base containing the store Class
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Class",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "StudyGroup", type: "string" }
	*        ]
	*    }]
	* });
	* // add some records to the store
	* base.store("Class").push({ Name: "Abed", StudyGroup: "A" });
	* base.store("Class").push({ Name: "Annie", StudyGroup: "B" });
	* base.store("Class").push({ Name: "Britta", StudyGroup: "C" });
	* base.store("Class").push({ Name: "Jeff", StudyGroup: "A" });
	* // change the StudyGroup of all records of store Class to A
	* base.store("Class").each(function (rec) { rec.StudyGroup = "A"; });	// all records in Class are now in study group A
	* base.close();
	*/
	//# exports.Store.prototype.each = function (callback) { return Object.create(require('qminer').Store.prototype); }
	JsDeclareFunction(each);

	/**
	* Creates an array of function outputs created from the store records.
	* @param {function} callback - Function that generates the array. It takes two parameters:
	* <br>rec - The current record.
	* <br>[idx] - The index of the current record.
	* @returns {Array<Object>} The array created by the callback function.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a base containing the store Class
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Class",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "StudyGroup", type: "string" }
	*        ]
	*    }]
	* });
	* // add some records to the store
	* base.store("Class").push({ Name: "Shirley", StudyGroup: "A" });
	* base.store("Class").push({ Name: "Troy", StudyGroup: "B" });
	* base.store("Class").push({ Name: "Chang", StudyGroup: "C" });
	* base.store("Class").push({ Name: "Pierce", StudyGroup: "A" });
	* // make an array of record names
	* var arr = base.store("Class").map(function (rec) { return rec.Name; }); // returns an array ["Shirley", "Troy", "Chang", "Pierce"]
	* base.close();
	*/
	//# exports.Store.prototype.map = function (callback) {}
	JsDeclareFunction(map);

	/**
	* Adds a record to the store.
	* @param {Object} rec - The added record. The record must be a JSON object corresponding to the store schema.
	* @param {boolean} [triggerEvents=true] - If true, all stream aggregate callbacks onAdd will be called after the record is inserted. If false, no stream aggregate will be updated.
	* @returns {number} The ID of the added record.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing two stores
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [
	*    {
	*        name: "Superheroes",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Superpowers", type: "string_v" }
	*        ]
	*    },
	*    {
	*        name: "Supervillians",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Superpowers", type: "string_v" }
	*        ]
	*    }]
	* });
	* // add a new superhero to the Superheroes store
	* base.store("Superheroes").push({ Name: "Superman", Superpowers: ["flight", "heat vision", "bulletproof"] }); // returns 0
	* // add a new supervillian to the Supervillians store
	* base.store("Supervillians").push({ Name: "Lex Luthor", Superpowers: ["expert engineer", "genius-level intellect", "money"] }); // returns 0
	* base.close();	
	*/
	//# exports.Store.prototype.push = function (rec, triggerEvents) { return 0; }
	JsDeclareFunction(push);

	/**
	* Creates a new record of given store. The record is not added to the store.
	* @param {Object} json - A JSON value of the record.
	* @returns {module:qm.Record} The record created by the JSON value and the store.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Planets",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Diameter", type: "int" },
	*            { name: "NearestStars", type: "string_v" }
	*        ]
	*    }]
	* });
	* // add a new planet in the store
	* base.store("Planets").push({ Name: "Earth", Diameter: 299196522, NearestStars: ["Sun"] });
	* // create a record of a planet (not added to the Planets store)
	* var planet = base.store("Planets").newRecord({ Name: "Tatooine", Diameter: 10465, NearestStars: ["Tatoo 1", "Tatoo 2"] });
	* base.close();
	*/
	//# exports.Store.prototype.newRecord = function (json) { return Object.create(require('qminer').Record.prototype); };
	JsDeclareFunction(newRecord);

	/**
	* Creates a new record set out of the records in store.
	* @param {module:la.IntVector} idVec - The integer vector containing the ids of selected records.
	* @returns {module:qm.RecordSet} The record set that contains the records gained with idVec.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Superheroes",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Superpowers", type: "string_v" }
	*        ]
	*    }]
	* });
	* // add some new records to the store
	* base.store("Superheroes").push({ Name: "Superman", Superpowers: ["Superhuman strength, speed, hearing", "Flight", "Heat vision"] });
	* base.store("Superheroes").push({ Name: "Batman", Superpowers: ["Genius-level intellect", "Peak physical and mental conditioning", "Master detective"] });
	* base.store("Superheroes").push({ Name: "Thor", Superpowers: ["Superhuman strength, endurance and longevity", "Abilities via Mjolnir"] });
	* base.store("Superheroes").push({ Name: "Wonder Woman", Superpowers: ["Superhuman strength, agility and endurance", "Flight", "Highly skilled hand-to-hand combatant"] });
	* // create a new record set containing only the DC Comic superheroes (those with the record ids 0, 1 and 3)
	* var intVec = new qm.la.IntVector([0, 1, 3]);
	* var DCHeroes = base.store("Superheroes").newRecordSet(intVec);
	* base.close();
	*/
	//# exports.Store.prototype.newRecordSet = function (idVec) { return Object.create(require('qminer').RecordSet.prototype); };
	JsDeclareFunction(newRecordSet);

	/**
	* Creates a record set containing random records from store.
	* @param {number} sampleSize - The size of the record set.
	* @returns {module:qm.RecordSet} Returns a record set containing random records.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "TVSeries",
	*        fields: [
	*            { name: "Title", type: "string", primary: true },
	*            { name: "NumberOfEpisodes", type: "int" }
	*        ]
	*    }]
	* });
	* // add some records in the store
	* base.store("TVSeries").push({ Title: "Archer", NumberOfEpisodes: 75 });
	* base.store("TVSeries").push({ Title: "The Simpsons", NumberOfEpisodes: 574 });
	* base.store("TVSeries").push({ Title: "New Girl", NumberOfEpisodes: 94 });
	* base.store("TVSeries").push({ Title: "Rick and Morty", NumberOfEpisodes: 11 });
	* base.store("TVSeries").push({ Title: "Game of Thrones", NumberOfEpisodes: 47 });
	* // create a sample record set containing 3 records
	* var randomRecordSet = base.store("TVSeries").sample(3); // contains 3 random records from the TVSeries store
	* base.close();
	*/
	//# exports.Store.prototype.sample = function (sampleSize) { return Object.create(require('qminer').RecordSet.prototype); };
	JsDeclareFunction(sample);

	/**
	* Gets the details of the selected field.
	* @param {string} fieldName - The name of the field.
	* @returns {Object} The JSON object containing the details of the field. 
	* @example
	* // import qm module
	* var qm = require("qminer");
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "People",
	*        fields: [
	*            { name: "Name", type: "string", primary: true },
	*            { name: "Gender", type: "string" },
	*            { name: "Age", type: "int" }
	*        ]
	*    }]
	* });
	* // get the details of the field "Name" of store "People"
	* // it returns a JSON object:
	* // { id: 0, name: "Name", type: "string", primary: true }
	* var details = base.store("People").field("Name");
	* base.close();
	*/
	//# exports.Store.prototype.field = function (fieldName) { return { id: 0, name:'', type:'', primary:'' }; }; 
	JsDeclareFunction(field);

	/**
	* Checks if the field is of numeric type.
	* @param {string} fieldName - The checked field.
	* @returns {boolean} True, if the field is of numeric type. Otherwise, false.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "TVSeries",
	*        fields: [
	*            { name: "Title", type: "string", primary: true },
	*            { name: "NumberOfEpisodes", type: "int" }
	*        ]
	*    }]
	* });
	* // check if the field "Title" is of numeric type
	* var isTitleNumeric = base.store("TVSeries").isNumeric("Title"); // returns false
	* // check if the field "NumberOfEpisodes" is of numeric type
	* var isNumberOfEpisodesNumeric = base.store("TVSeries").isNumeric("NumberOfEpisodes"); // returns true
	* base.close();
	*/
	//# exports.Store.prototype.isNumeric = function (fieldName) { return true; };
	JsDeclareFunction(isNumeric)

	/**
	* Checks if the field is of string type.
	* @param {string} fieldName - The checked field.
	* @returns {boolean} True, if the field is of string type. Otherwise, false.
	* @example
	* // import qm module
	* var qm = require("qminer");
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "People",
	*        fields: [
	*            { name: "Name", type: "string", primary: true },
	*            { name: "Gender", type: "string" },
	*            { name: "Age", type: "int" }
	*        ]
	*    }]
	* });
	* // check if the field "Name" is of string type
	* var isNameString = base.store("People").isString("Name"); // returns true
	* // check if the field "Age" is of string type
	* var isAgeString = base.store("People").isString("Age"); // returns false
	* base.close();
	*/
	//# exports.Store.prototype.isString = function (fieldName) { return true; }; 
	JsDeclareFunction(isString)

	/**
	* Checks if the field is of type Date.
	* @param {string} fieldName - The checked field.
	* @returns {boolean} True, if the field is of type Date. Otherwise, false.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "BasketballPlayers",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "SeasonScore", type: "int_v" },
	*            { name: "DateOfBirth", type: "datetime" }
	*        ]
	*    }]
	* });
	* // check if the SeasonScore field is of type Date
	* var isSeasonScoreDate = base.store("BasketballPlayers").isDate("SeasonScore"); // returns false
	* // check if the FirstPlayed field is of type Date
	* var isFirstPlayedDate = base.store("BasketballPlayers").isDate("DateOfBirth"); // returns true
	* base.close();
	*/
	//# exports.Store.prototype.isDate = function (fieldName) { return true; }
	JsDeclareFunction(isDate)

	/**
	* Returns the details of the selected key as a JSON object.
	* @param {string} keyName - The selected key as a JSON object.
	* @returns {Object} The JSON object containing the details of the key.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Countries",
	*        fields: [
	*            { name: "Name", type: "string", primary: true },
	*            { name: "Population", type: "int" },
	*            { name: "Continent", type: "string" }
	*        ],
	*        keys: [
	*            { field: "Name", type: "text" },
	*            { field: "Continent", type: "value" }
	*        ]
	*    }]
	* });
	* // get the details of the key of the field "Continent"
	* // returns a JSON object containing the details of the key:
	* // { fq: { length: 0 }, vocabulary: { length: 0 }, name: 'Continent', store: { name: 'Countries', ... }}
	* var details = base.store("Countries").key("Continent");
	* base.close();
	*/
	//# exports.Store.prototype.key = function (keyName) { return { fq: {}, vocabulary: {}, name:'', store: {} }; };
	JsDeclareFunction(key);

	////!- `store.addTrigger(trigger)` -- add `trigger` to the store triggers. Trigger is a JS object with three properties `onAdd`, `onUpdate`, `onDelete` whose values are callbacks
	//JsDeclareFunction(addTrigger); Deprecated - use new qm.sa(...) instead

	/**
	* Returns the stream aggregate with the given name.
	* @param {string} saName - The name of the stream aggregate.
	* @returns {module:qm.StreamAggr} The stream aggregate with the saName.
	*/
	//# exports.Store.prototype.getStreamAggr = function (saName) { return Object.create(require('qminer').StreamAggr.prototype); }
	JsDeclareFunction(getStreamAggr);

	/**
	* Resets all stream aggregates.
	*/
	//# exports.Store.prototype.resetStreamAggregates = function () { }
	JsDeclareFunction(resetStreamAggregates);

	/**
	* Returns an array of the stream aggregates names connected to the store.
	* @returns {Array.<string>} An array of stream aggregates names.
	*/
	//# exports.Store.prototype.getStreamAggrNames = function () { return [""]; }
	JsDeclareFunction(getStreamAggrNames);

	/**
	* Returns the store as a JSON.
	* @returns {Object} The store as a JSON.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "FootballPlayers",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "FootballClubs", type: "string_v" },
	*            { name: "GoalsPerSeason", type: "int_v" },
	*        ]
	*    }]
	* });
	* // get the store as a JSON object
	* // the returned JSON object is:
	* // { storeId: 0, storeName: 'FootballPlayers', storeRecords: 0, fields: [...], keys: [], joins: [] }
	* var json = base.store("FootballPlayers").toJSON();
	* base.close();
	*/
	//# exports.Store.prototype.toJSON = function () { return { storeId:0, storeName:'', storeRecords:'', fields:[], keys:[], joins:[] }; };
	JsDeclareFunction(toJSON);

	/**
	* Deletes the records in the store.
	* @param {number} [num] - The number of deleted records. If the number is given, the first num records will be deleted.
	* @returns {number} The number of remaining records in the store.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "TVSeries",
	*        fields: [
	*            { name: "Title", type: "string", primary: true },
	*            { name: "NumberOfEpisodes", type: "int" }
	*        ]
	*    }]
	* });
	* // add some records in the store
	* base.store("TVSeries").push({ Title: "Archer", NumberOfEpisodes: 75 });
	* base.store("TVSeries").push({ Title: "The Simpsons", NumberOfEpisodes: 574 });
	* base.store("TVSeries").push({ Title: "New Girl", NumberOfEpisodes: 94 });
	* base.store("TVSeries").push({ Title: "Rick and Morty", NumberOfEpisodes: 11 });
	* base.store("TVSeries").push({ Title: "Game of Thrones", NumberOfEpisodes: 47 });
	* // deletes the first 2 records (Archer and The Simpsons) in TVSeries
	* base.store("TVSeries").clear(2); // returns 3
	* // delete all remaining records in TVStore
	* base.store("TVSeries").clear();  // returns 0
	* base.close();
	*/
	//# exports.Store.prototype.clear = function (num) { return 0; };
	JsDeclareFunction(clear);

	/**
	* Gives a vector containing the field value of each record.
	* @param {string} fieldName - The field name. Field must be of one-dimensional type, e.g. int, float, string...
	* @returns {module:la.Vector} The vector containing the field values of each record.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Companies",
	*        fields: [
	*            { name: "Name", type: "string", primary: true },
	*            { name: "Location", type: "string" }
	*        ]
	*    }]
	* });
	* // add some records to the store
	* base.store("Companies").push({ Name: "DC Comics", Location: "Burbank, California" });
	* base.store("Companies").push({ Name: "DC Shoes", Location: "Huntington Beach, California" });
	* base.store("Companies").push({ Name: "21st Century Fox", Location: "New York City, New York" });
	* // get the vector of company names
	* var companyNames = base.store("Companies").getVector("Name");	// returns a vector ["DC Comics", "DC Shoes", "21st Century Fox"]
	* base.close();
	*/
	//# exports.Store.prototype.getVector = function (fieldName) { return Object.create(require('qminer').la.Vector.prototype); };
	JsDeclareFunction(getVector);

	/**
	* Gives a matrix containing the field values of each record.
	* @param {string} fieldName - The field name. Field mustn't be of type string.
	* @returns {(module:la.Matrix | module:la.SparseMatrix)} The matrix containing the field values. 
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "ArcheryChampionship",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "ScorePerRound", type: "float_v" }
	*        ]
	*    }]
	* });
	* // set new records in the store
	* base.store("ArcheryChampionship").push({ Name: "Robin Hood", ScorePerRound: [50, 48, 48] });
	* base.store("ArcheryChampionship").push({ Name: "Oliver Queen", ScorePerRound: [44, 46, 44] });
	* base.store("ArcheryChampionship").push({ Name: "Legolas", ScorePerRound: [50, 50, 48] });
	* // get the matrix containing the "score per round" values
	* // The values of the i-th column are the values of the i-th record.
	* // The function will give the matrix:
	* // 50  44  50
	* // 48  46  50
	* // 48  44  48
	* var matrix = base.store("ArcheryChampionship").getMatrix("ScorePerRound");
	* base.close();
	*/
	//# exports.Store.prototype.getMatrix = function (fieldName) { return Object.create(require('qminer').la.Matrix.prototype); };
	JsDeclareFunction(getMatrix);

	/**
	* Gives the field value of a specific record.
	* @param {number} recId - The record id.
	* @param {string} fieldName - The field's name.
	* @returns {Object} The fieldName value of the record with recId.
	* @example
	* //import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Festivals",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Type", type: "string" },
	*            { name: "Location", type: "string" }
	*        ]
	*    }]
	* });
	* // add some records in the store
	* base.store("Festivals").push({ Name: "Metaldays", Type: "music", Location: "Tolmin, Slovenia" });
	* base.store("Festivals").push({ Name: "Festival de Cannes", Type: "movie", Location: "Cannes, France" });
	* base.store("Festivals").push({ Name: "The Festival of Chocolate", Type: "food", Location: "Hillsborough, USA" });
	* // get the field value of the second record for field "Type"
	* var fieldValue = base.store("Festivals").cell(1, "Type"); // returns "movie"
	* base.close();
	*/
	//# exports.Store.prototype.cell = function (recId, fieldName) {};
	JsDeclareFunction(cell);

	/**
	* Calls onAdd callback on all stream aggregates
	* @param {(module:qm.Record | number)} [arg=this.last] - The record or recordId which will be passed to onAdd callbacks. If the record or recordId is not provided, the last record will be used. Throws exception if cannot be provided.
	*/
	//# exports.Store.prototype.triggerOnAddCallbacks = function (arg) {};
	JsDeclareFunction(triggerOnAddCallbacks);

	/**
	* Gives the name of the store.
	*/
	//# exports.Store.prototype.name = "";
	JsDeclareProperty(name);

	/**
	* Checks if the store is empty.
	*/
	//# exports.Store.prototype.empty = true;
	JsDeclareProperty(empty);

	/**
	* Gives the number of records.
	*/
	//# exports.Store.prototype.length = 0;
	JsDeclareProperty(length);

	/**
	* Creates a record set containing all the records from the store.
	*/
	//# exports.Store.prototype.allRecords = Object.create(require('qminer').RecordSet.prototype);
	JsDeclareFunction(allRecords);

	/**
	* Gives an array of all field descriptor JSON objects.
	*/
	//# exports.Store.prototype.fields = undefinied;
	JsDeclareProperty(fields);

	/**
	* Gives an array of all join descriptor JSON objects.
	*/
	//# exports.Store.prototype.joins = undefined;
	JsDeclareProperty(joins);

	/**
	* Gives an array of all key descriptor JSON objects.
	*/
	//# exports.Store.prototype.keys = undefined;
	JsDeclareProperty(keys);

	/**
	* Returns the first record of the store.
	*/
	//# exports.Store.prototype.first = Object.create(require('qminer').Record.prototype);
	JsDeclareFunction(first);

	/**
	* Returns the last record of the store.
	*/
	//# exports.Store.prototype.last = Object.create(require('qminer').Record.prototype);
	JsDeclareFunction(last);

	/**
	* Returns an iterator for iterating over the store from start to end.
	*/
	//# exports.Store.prototype.forwardIter = Object.create(require('qminer').Iterator.prototype);
	JsDeclareFunction(forwardIter);

	/**
	* Returns an iterator for iterating over the store form end to start.
	*/
	//# exports.Store.prototype.backwardIter = Object.create(require('qminer').Iterator.prototype);
	JsDeclareFunction(backwardIter);

	//!- `rec = store[recId]` -- get record with ID `recId`; 
	//!     returns `null` when no such record exists
	/**
	* Gets the record with the given ID.
	* @param {number} recId - The id of the record.
	* @returns {module:qm.Record} The record with the ID equal to recId.
	* @ignore
	*/
	//# exports.Store.prototype.store = function (recId) { };
	JsDeclIndexedProperty(indexId);	

	/**
	* Returns the base, in which the store is contained.
	*/
	//# exports.Store.prototype.base = Object.create(require('qminer').Base.prototype);
	JsDeclareFunction(base);
	//!JSIMPLEMENT:src/qminer/store.js
};

///////////////////////////////
// NodeJs QMiner Record

/**
* Records are used for storing data 
* in {@link module:qm.Store}.
* <b>Factory pattern</b>: this class cannot be construced using the new keyword. This class is constructed
* when calling a specific method or attribute, e.g. using {@link module:qm.Store#push} to create a new record in 
* the store or {@link module:qm.Store#newRecord} to create a new record, that is not saved in the store.
* @class
*/
//# exports.Record = function () {}; 
class TNodeJsRec: public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	// Modified node framework: one record template per each base,storeId combination 
	static TVec<TVec<v8::Persistent<v8::Function> > > BaseStoreIdConstructor;
public:
	// Node framework 
	static void Init(const TWPt<TQm::TStore>& Store);
	static const TStr GetClassId() { return "TRec"; }
	// when reseting a db we have to clear the old record templates
	static void Clear(const int& BaseId);
	// Object that knows if Base is valid
	PNodeJsBaseWatcher Watcher;
	// C++ wrapped object
	TQm::TRec Rec;
	TInt Fq;
	// C++ constructors	
	TNodeJsRec(PNodeJsBaseWatcher _Watcher, const TQm::TRec& _Rec, const TInt& _Fq = 1) : Watcher(_Watcher), Rec(_Rec), Fq(_Fq) {}
	// Not typical (records have multiple templates), simpler objects get this method from TNodeJsUtil
	static v8::Local<v8::Object> NewInstance(TNodeJsRec* Obj);
	
private:

	/**
	* Clones the record.
	* @returns {module:qm.Record} The clone of the record.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "StarWarsMovies",
	*        fields: [
	*            { name: "Title", type: "string" },
	*            { name: "ReleseDate", type: "datetime" },
	*            { name: "Length", type: "int" }
	*        ]
	*    }]
	* });
	* // create some records in the new store
	* base.store("StarWarsMovies").push({ Title: "Attack of the Clones", ReleseDate: "2002-05-16T00:00:00", Length: 142 });
	* base.store("StarWarsMovies").push({ Title: "The Empire Strikes Back", ReleseDate: "1980-06-20T00:00:00", Length: 124 });
	* base.store("StarWarsMovies").push({ Title: "Return of the Jedi", ReleseDate: "1983-05-25T00:00:00", Length: 134 });
	* // create a clone of the "Attack of the Clones" record
	* var clone = base.store("StarWarsMovies")[0].$clone();
	* base.close();
	*/
	//# exports.Record.prototype.$clone = function () { return Object.create(require('qminer').Record.prototype); };
    JsDeclareFunction(clone);
	
	/**
	* Adds a join record `joinRecord` to join `jonName` (string) with join frequency `joinFrequency`
	* @param {string} joinName - join name
	* @param {(module:qm.Record | number)} joinRecord - joined record or its ID
	* @param {number} [joinFrequency=1] - frequency attached to the join
	* @returns {module:qm.Record} Record.
	*/
	//# exports.Record.prototype.$addJoin = function (joinName, joinRecord, joinFrequency) { return Object.create(require('qminer').Record.prototype); }
	JsDeclareFunction(addJoin);

	/**
	* Deletes join record `joinRecord` from join `joinName` (string) with join frequency `joinFrequency`.
	* @param {string} joinName - join name
	* @param {(module:qm.Record | number)} joinRecord - joined record or its ID
	* @param {number} [joinFrequency=1] - frequency attached to the join
	* @returns {module:qm.Record} Record.
	*/
	//# exports.Record.prototype.$delJoin = function (joinName, joinRecord, joinFrequency) { return Object.create(require('qminer').Record.prototype); }
	JsDeclareFunction(delJoin);

	/**
	* Creates a JSON version of the record.
	*
    * @param {Boolean} [joinRecords=false] - include joined records (only IDs)
    * @param {Boolean} [joinRecordFields=false] - expand joined record fields
    * @param {Boolean} [sysFields=true] - if set to true system fields, like $id, will be included
	* @returns {Object} The JSON version of the record.
	*
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Musicians",
	*        fields: [
	*            { name: "Name", type: "string", primary: true },
	*            { name: "DateOfBirth", type: "datetime" },
	*            { name: "GreatestHits", type: "string_v" }
	*        ]
	*    }]
	* });
	* // create some records
	* base.store("Musicians").push({ Name: "Jimmy Page", DateOfBirth:  "1944-01-09T00:00:00", GreatestHits: ["Stairway to Heaven", "Whole Lotta Love"] });
	* base.store("Musicians").push({ Name: "Beyonce", DateOfBirth: "1981-09-04T00:00:00", GreatestHits: ["Single Ladies (Put a Ring on It)"] });
	* // get a JSON version of the "Beyonce" record 
	* // The JSON object for this example si:
	* // { '$id': 1, Name: 'Beyonce', ActiveSince: '1981-09-04T00:00:00', GreatestHits: ['Single Ladies (Put a Ring on It)'] }
	* var json = base.store("Musicians").recordByName("Beyonce").toJSON();
	* base.close();
	*/
	//# exports.Record.prototype.toJSON = function () {};
    JsDeclareFunction(toJSON);

	/**
	* Returns the id of the record.
	*/
	//# exports.Record.prototype.$id = 0;
	JsDeclareProperty(id);

	/**
	* Returns the name of the record.
	*/
	//# exports.Record.prototype.$name = "";
	JsDeclareProperty(name);

	/**
	* Returns the frequency of the record.
	*/
	//# exports.Record.prototype.$fq = 0;
	JsDeclareProperty(fq);

	/**
	* Returns the store the record belongs to.
	*/
	//# exports.Record.prototype.store = Object.create('qminer').Store.prototype;
	JsDeclareFunction(store);

	JsDeclareSetProperty(getField, setField);
	JsDeclareFunction(join);
	JsDeclareFunction(sjoin);
};

class TNodeJsRecByValV: public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	// Node framework
	static v8::Persistent<v8::Function> Constructor;
public:
	// Node framework
	static void Init(v8::Handle<v8::Object> Exports);
	static const TStr GetClassId() { return "RecordVector"; }

	TVec<TQm::TRec> RecV;

	TNodeJsRecByValV(): RecV() {}

	static TNodeJsRecByValV* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

private:
	/**
	 * Adds a new record to the vector.
	 */
	//# exports.RecVector.prototype.push = function (rec) {};
	JsDeclareFunction(push);
};

///////////////////////////////
// NodeJs QMiner Record Set

/**
* Record Set is a set of records.
* <b>Factory pattern</b>: this class cannot be construced using the new keyword. This class is constructed
* when calling a specific method or attribute, e.g. using {@link module:qm.Store#allRecords} to get all the records
* in the store as a record set.
* @class
*/
//# exports.RecordSet = function () {}
class TNodeJsRecSet: public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	// Node framework
	static v8::Persistent<v8::Function> Constructor;
public:
	// Node framework 
	static void Init(v8::Handle<v8::Object> exports);
	static const TStr GetClassId() { return "RecSet"; }
	// C++ wrapped object
	TQm::PRecSet RecSet;
	// Object that knows if Base is valid
	PNodeJsBaseWatcher Watcher;
	// C++ constructors
	TNodeJsRecSet(const TQm::PRecSet& _RecSet, PNodeJsBaseWatcher& _Watcher) : RecSet(_RecSet), Watcher(_Watcher) {}
private:

	/**
	* Creates a new instance of the record set.
	* @returns {module:qm.RecordSet} A copy of the record set.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Philosophers",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Era", type: "string" }
	*        ]
	*    }]
	* });
	* // put some records in the store
	* base.store("Philosophers").push({ Name: "Plato", Era: "Ancient philosophy" });
	* base.store("Philosophers").push({ Name: "Immanuel Kant", Era: "18th-century philosophy" });
	* base.store("Philosophers").push({ Name: "Emmanuel Levinas", Era: "20th-century philosophy" });
	* base.store("Philosophers").push({ Name: "Rene Descartes", Era: "17th-century philosophy" });
	* base.store("Philosophers").push({ Name: "Confucius", Era: "Ancient philosophy" });
	* // create a record set out of the records in store
	* var recordSet = base.store("Philosophers").allRecords;
	* // clone the record set of the "Philosophers" store
	* var philosophers = recordSet.clone();
	* base.close();
	*/
	//# exports.RecordSet.prototype.clone = function () { return Object.create(require(qminer).RecordSet.prototype); };
	JsDeclareFunction(clone);

	/**
	* Creates a new record set out of the join attribute of records.
	* @param {string} joinName - The name of the join attribute.
	* @param {number} [sampleSize] - The number of records to be used for construction of the record set.
	* @returns {module:qm.RecordSet} The record set containing the join records.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing two stores, with join attributes
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [
	*    {
	*        name: "Musicians",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Instruments", type: "string_v" }
	*        ],
	*        joins: [
	*            { name: "PlaysIn", type: "index", store: "Bands", inverse: "Members" }
	*        ]
	*    },
	*    {
	*        name: "Bands",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Genre", type: "string" }
	*        ],
	*        joins: [
	*            { name: "Members", type: "index", store: "Musicians", inverse: "PlaysIn" }
	*        ]
	*    }]
	* });
	* // add some new records to both stores
	* base.store("Musicians").push({ Name: "Robert Plant", Instruments: ["Vocals"], PlaysIn: [{Name: "Led Zeppelin", "Genre": "Rock" }] });
	* base.store("Musicians").push({ Name: "Jimmy Page", Instruments: ["Guitar"], PlaysIn: [{Name: "Led Zeppelin", "Genre": "Rock" }] });
	* base.store("Bands").push({ Name: "The White Stripes", Genre: "Rock" });
	* // create a record set containing the musicians, that are members of some bend
	* // returns a record set containing the records of "Robert Plant" and "Jimmy Page"
	* var ledZeppelin = base.store("Bands").allRecords.join("Members");
	* // create a record set containing the first musician, that is a member of some band
	* // returns a record set containing only one record, which is "Robert Plant" or "Jimmy Page"
	* var ledMember = base.store("Bands").allRecords.join("Members", 1);
	* base.close();
	*/
	//# exports.RecordSet.prototype.join = function (joinName, sampleSize) { return Object.create(require('qminer').RecordSet.prototype); };
	JsDeclareFunction(join);

	/**
	* Aggr // TODO
	* @param {Object} [aggrQueryJSON] 
	* @returns {Object} Aggregate
	* @ignore
	*/
	//# exports.RecordSet.prototype.aggr = function (aggrQueryJSON) {};
	JsDeclareFunction(aggr);
    
	/**
	* Truncates the first records.
	* @param {number} limit_num - How many records to truncate.
	* @param {number} [offset_num] - Where to start to truncate.
	* @returns {module:qm.RecordSet} Self.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Philosophers",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Era", type: "string" }
	*        ]
	*    }]
	* });
	* // put some records in the store
	* base.store("Philosophers").push({ Name: "Plato", Era: "Ancient philosophy" });
	* base.store("Philosophers").push({ Name: "Immanuel Kant", Era: "18th-century philosophy" });
	* base.store("Philosophers").push({ Name: "Emmanuel Levinas", Era: "20th-century philosophy" });
	* base.store("Philosophers").push({ Name: "Rene Descartes", Era: "17th-century philosophy" });
	* base.store("Philosophers").push({ Name: "Confucius", Era: "Ancient philosophy" });
	* // create two identical record sets of the "Philosophers" store
	* var recordSet1 = base.store("Philosophers").allRecords;
	* var recordSet2 = base.store("Philosophers").allRecords;
	* // truncate the first 3 records in recordSet1
	* recordSet1.trunc(3); // return self, containing only the first 3 records ("Plato", "Immanuel Kant", "Emmanuel Levinas")
	* // truncate the first 2 records in recordSet2, starting with "Emmanuel Levinas"
	* recordSet2.trunc(2, 2); // returns self, containing only the 2 records ("Emmanuel Levinas", "Rene Descartes")
	* base.close();
	*/
	//# exports.RecordSet.prototype.trunc = function (limit_num, offset_num) { return Object.create(require('qminer').RecordSet.prototype); };
	JsDeclareFunction(trunc);

	/**
	* Creates a random sample of records of the record set.
	* @param {number} num - The number of records in the sample.
	* @returns {module:qm.RecordSet} A record set containing the sample records.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base with one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Movies",
	*        fields: [
	*            { name: "Title", type: "string" },
	*            { name: "Length", type: "int" },
	*            { name: "Director", type: "string" }
	*        ]
	*    }]
	* });
	* // put some records in the store
	* base.store("Movies").push({ Title: "The Nightmare Before Christmas", Length: 76, Director: "Henry Selick" });
	* base.store("Movies").push({ Title: "Jurassic Part", Length: 127, Director: "Steven Spielberg" });
	* base.store("Movies").push({ Title: "The Avengers", Length: 143, Director: "Joss Whedon" });
	* base.store("Movies").push({ Title: "The Clockwork Orange", Length: 136, Director: "Stanley Kubrick" });
	* base.store("Movies").push({ Title: "Full Metal Jacket", Length: 116, Director: "Stanely Kubrick" });
	* // create a sample record set of containing 3 records from the "Movies" store
	* var sample = base.store("Movies").allRecords.sample(3);
	* base.close();
	*/
	//# exports.RecordSet.prototype.sample = function (num) { return Object.create(require('qminer').RecordSet.prototype); };
	JsDeclareFunction(sample);

	/**
	* Shuffles the order of records in the record set.
	* @param {number} [seed] - Integer.
	* @returns {module:qm.RecordSet} Self. The records in the record set are in a different order.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "WeatherForcast",
	*        fields: [
	*            { name: "Weather", type: "string" },
	*            { name: "Date", type: "datetime" },
	*            { name: "TemperatureDegrees", type: "int" }
	*        ]
	*    }]
	* });
	* // put some records in the "WeatherForecast" store
	* base.store("WeatherForcast").push({ Weather: "Partly Cloudy", Date: "2015-05-27T11:00:00", TemperatureDegrees: 19 });
	* base.store("WeatherForcast").push({ Weather: "Partly Cloudy", Date: "2015-05-28T11:00:00", TemperatureDegrees: 22 });
	* base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-05-29T11:00:00", TemperatureDegrees: 25 });
	* base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-05-30T11:00:00", TemperatureDegrees: 25 });
	* base.store("WeatherForcast").push({ Weather: "Scattered Showers", Date: "2015-05-31T11:00:00", TemperatureDegrees: 24 });
	* base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-06-01T11:00:00", TemperatureDegrees: 27 });
	* // get the record set containing the records from the "WeatherForcast" store
	* var recordSet = base.store("WeatherForcast").allRecords;
	* // shuffle the records in the newly created record set. Use the number 100 as the seed for the shuffle
	* recordSet.shuffle(100); // returns self, the records in the record set are shuffled
	* base.close();
	*/
	//# exports.RecordSet.prototype.shuffle = function (seed) { return Object.create(require('qminer').RecordSet.prototype); };
	JsDeclareFunction(shuffle);

	/**
	* It reverses the record order.
	* @returns {module:qm.RecordSet} Self. Records are in reversed order.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "WeatherForcast",
	*        fields: [
	*            { name: "Weather", type: "string" },
	*            { name: "Date", type: "datetime" },
	*            { name: "TemperatureDegrees", type: "int" },
	*        ]
	*    }]
	* });
	* // put some records in the "WeatherForecast" store
	* base.store("WeatherForcast").push({ Weather: "Partly Cloudy", Date: "2015-05-27T11:00:00", TemperatureDegrees: 19 });
	* base.store("WeatherForcast").push({ Weather: "Partly Cloudy", Date: "2015-05-28T11:00:00", TemperatureDegrees: 22 });
	* base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-05-29T11:00:00", TemperatureDegrees: 25 });
	* base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-05-30T11:00:00", TemperatureDegrees: 25 });
	* base.store("WeatherForcast").push({ Weather: "Scattered Showers", Date: "2015-05-31T11:00:00", TemperatureDegrees: 24 });
	* base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-06-01T11:00:00", TemperatureDegrees: 27 });
	* // get the record set containing the records from the "WeatherForcast" store
	* var recordSet = base.store("WeatherForcast").allRecords;
	* // reverse the record order in the record set
	* recordSet.reverse(); // returns self, the records in the record set are in the reverse order
	* base.close();
	*/
	//# exports.RecordSet.prototype.reverse = function () { return Object.create(require('qminer').RecordSet.prototype); };
	JsDeclareFunction(reverse);

	/**
	* Sorts the records according to record id.
	* @param {number} [asc=-1] - If asc > 0, it sorts in ascending order. Otherwise, it sorts in descending order.  
	* @returns {module:qm.RecordSet} Self. Records are sorted according to record id and asc.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Tea",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Type", type: "string"},
	*            { name: "Origin", type: "string", "null": true  }
	*        ]
	*    }]
	* });
	* // put some records in the "Tea" store
	* base.store("Tea").push({ Name: "Tanyang Gongfu", Type: "Black", Origin: "Tanyang" });
	* base.store("Tea").push({ Name: "Rou Gui", Type: "White" });
	* base.store("Tea").push({ Name: "Tieluohan Tea", Type: "Wuyi", Origin: "Northern Fujian" });
	* base.store("Tea").push({ Name: "Red Robe", Type: "Oolong", Origin: "Wuyi Mountains" });
	* // get the records of the "Tea" store as a record set
	* var recordSet = base.store("Tea").allRecords;
	* // sort the records in the record set by their id in descending order
	* recordSet.sortById(); // returns self, the records are sorted in descending order (default)
	* // sort the records in the record set by their id in ascending order
	* recordSet.sortById(1); // returns self, the records are sorted in ascending order
	* base.close();
	*/
	//# exports.RecordSet.prototype.sortById = function (asc) { return Object.create(require('qminer').RecordSet.prototype); }; 
	JsDeclareFunction(sortById);

	/**
	* Sorts the records according to their weight.
	* @param {number} [asc=1] - If asc > 0, it sorts in ascending order. Otherwise, it sorts in descending order.
	* @returns {module:qm.RecordSet} Self. Records are sorted according to record weight and asc.
	*/
	//# exports.RecordSet.prototype.sortByFq = function (asc) { return Object.create(require('qminer').RecordSet.prototype); }; 
	JsDeclareFunction(sortByFq);

	/**
	* Sorts the records according to a specific record field.
	* @param {string} fieldName - The field by which the sort will work.
	* @param {number} [arc=-1] - if asc > 0, it sorts in ascending order. Otherwise, it sorts in descending order.
	* @returns {module:qm.RecordSet} Self. Records are sorted according to fieldName and arc.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "TVSeries",
	*        fields: [
	*            { name: "Title", type: "string", primary: true },
	*            { name: "NumberOfEpisodes", type: "int" }
	*        ]
	*    }]
	* });
	* // add some records in the store
	* base.store("TVSeries").push({ Title: "Archer", NumberOfEpisodes: 75 });
	* base.store("TVSeries").push({ Title: "The Simpsons", NumberOfEpisodes: 574 });
	* base.store("TVSeries").push({ Title: "New Girl", NumberOfEpisodes: 94 });
	* base.store("TVSeries").push({ Title: "Rick and Morty", NumberOfEpisodes: 11 });
	* base.store("TVSeries").push({ Title: "Game of Thrones", NumberOfEpisodes: 47 });
	* // get the records of the "TVSeries" store as a record set
	* var recordSet = base.store("TVSeries").allRecords;
	* // sort the records by their "Title" field in ascending order 
	* recordSet.sortByField("Title", true); // returns self, record are sorted by their "Title"
	* base.close();
	*/
	//# exports.RecordSet.prototype.sortByField = function (fieldName, asc) { return Object.create(require('qminer').RecordSet.prototype); };
	JsDeclareFunction(sortByField);

	/**
	* Sorts the records according to the given callback function.
	* @param {function} callback - The function used to sort the records. It takes two parameters:
	* <br>1. rec - The first record.
	* <br>2. rec2 - The second record.
	* <br>It returns a boolean object.
	* @returns {module:qm.RecordSet} Self. The records are sorted according to the callback function.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "TVSeries",
	*        fields: [
	*            { name: "Title", type: "string", primary: true },
	*            { name: "NumberOfEpisodes", type: "int" }
	*        ]
	*    }]
	* });
	* // add some records in the store
	* base.store("TVSeries").push({ Title: "Archer", NumberOfEpisodes: 75 });
	* base.store("TVSeries").push({ Title: "The Simpsons", NumberOfEpisodes: 574 });
	* base.store("TVSeries").push({ Title: "New Girl", NumberOfEpisodes: 94 });
	* base.store("TVSeries").push({ Title: "Rick and Morty", NumberOfEpisodes: 11 });
	* base.store("TVSeries").push({ Title: "Game of Thrones", NumberOfEpisodes: 47 });
	* // get the records of the "TVSeries" store as a record set
	* var recordSet = base.store("TVSeries").allRecords;
	* // sort the records by their number of episodes
	* recordSet.sort(function (rec, rec2) { return rec.NumberOfEpisodes < rec2.NumberOfEpisodes; }); // returns self, records are sorted by the number of episodes
	* base.close();
	*/
	//# exports.RecordSet.prototype.sort = function (callback) { return Object.create(require('qminer').RecordSet.prototype); };
	JsDeclareFunction(sort);

	/**
	* Keeps only records with ids between or equal two values.
	* @param {number} [minId] - The minimum id.
	* @param {number} [maxId] - The maximum id.
	* @returns {module:qm.RecordSet} Self. 
	* <br>1. Contains only the records of the original with ids between minId and maxId, if parameters are given.
	* <br>2. Contains all the records of the original, if no parameter is given.
	* @example
	* // import qm require
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "FrankSinatraGreatestHits",
	*        fields: [
	*            { name: "Title", type: "string" },
	*            { name: "Length", type: "int" }
	*        ]
	*    }]
	* });
	* // put some records in the "FrankSinatraGreatesHits" store
	* base.store("FrankSinatraGreatestHits").push({ Title: "Strangers in the Night", Length: 145 });
	* base.store("FrankSinatraGreatestHits").push({ Title: "Summer Wind", Length: 173 });
	* base.store("FrankSinatraGreatestHits").push({ Title: "It Was a Very Good Year", Length: 265 });
	* base.store("FrankSinatraGreatestHits").push({ Title: "Somewhere in Your Heart", Length: 146 });
	* base.store("FrankSinatraGreatestHits").push({ Title: "Forget Domani", Length: 156 });
	* base.store("FrankSinatraGreatestHits").push({ Title: "Somethin' Stupid", Length: 155 });
	* base.store("FrankSinatraGreatestHits").push({ Title: "This Town", Length: 186 });
	* // get the records of the store as a record set
	* var recordSet = base.store("FrankSinatraGreatestHits").allRecords;
	* // from the record set keep the records with indeces between or equal 2 and 5
	* recordSet.filterById(2, 5);
	* base.close();
	*/
	//# exports.RecordSet.prototype.filterById = function (minId, maxId) { return Object.create(require('qminer').RecordSet.prototype); };
	JsDeclareFunction(filterById);
	
	/**
	* Keeps only the records with weight between two values.
	* @param {number} [minFq] - The minimum value.
	* @param {number} [maxFq] - The maximum value.
	* @returns {module:qm.RecordSet} Self.
	* <br>1. Contains only the records of the original with weights between minFq and maxFq, if parameters are given.
	* <br>2. Contains all the records of the original, if no parameter is given.
	* @ignore
	*/
	//# exports.RecordSet.prototype.filterByFq = function (minFq, maxFq) { return Object.create(require('qminer').RecordSet.prototype); };
	JsDeclareFunction(filterByFq);

	/**
	* Keeps only the records with a specific value of some field.
	* @param {string} fieldName - The field by which the records will be filtered.
	* @param {(string | number)} minVal -
	* <br>1. Is a string, if the field type is a string. The exact string to compare.
	* <br>2. Is a number, if the field type is a number. The minimal value for comparison.
	* <br>3. TODO Time field
	* @param {number} maxVal - Only in combination with minVal for non-string fields. The maximal value for comparison.
	* @returns {module:qm.RecordSet} Self. Containing only the records with the fieldName value between minVal and maxVal. If the fieldName type is string,
	* it contains only the records with fieldName equal to minVal.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "WeatherForcast",
	*        fields: [
	*            { name: "Weather", type: "string" },
	*            { name: "Date", type: "datetime" },
	*            { name: "TemperatureDegrees", type: "int" },
	*        ]
	*    }]
	* });
	* // put some records in the "WeatherForecast" store
	* base.store("WeatherForcast").push({ Weather: "Partly Cloudy", Date: "2015-05-27T11:00:00", TemperatureDegrees: 19 });
	* base.store("WeatherForcast").push({ Weather: "Partly Cloudy", Date: "2015-05-28T11:00:00", TemperatureDegrees: 22 });
	* base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-05-29T11:00:00", TemperatureDegrees: 25 });
	* base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-05-30T11:00:00", TemperatureDegrees: 25 });
	* base.store("WeatherForcast").push({ Weather: "Scattered Showers", Date: "2015-05-31T11:00:00", TemperatureDegrees: 24 });
	* base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-06-01T11:00:00", TemperatureDegrees: 27 });
	* // get the record set containing the records from the "WeatherForcast" store
	* var recordSet = base.store("WeatherForcast").allRecords;
	* // filter only the records, where the weather is Mostly Cloudy
	* recordSet.filterByField("Weather", "Mostly Cloudy"); // returns self, containing only the records, where the weather is "Mostly Cloudy"
	* base.close();
	*/
	//# exports.RecordSet.prototype.filterByField = function (fieldName, minVal, maxVal) { return Object.create(require('qminer').RecordSet.prototype); };
	JsDeclareFunction(filterByField);

	/**
	* Keeps only the records that pass the callback function.
	* @param {function} callback - The filter function. It takes one parameter and return a boolean object.
	* @returns {module:qm.RecordSet} Self. Containing only the record that pass the callback function.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "ArcheryChampionship",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "ScorePerRound", type: "float_v" }
	*        ]
	*    }]
	* });
	* // set new records in the store
	* base.store("ArcheryChampionship").push({ Name: "Robin Hood", ScorePerRound: [50, 48, 48] });
	* base.store("ArcheryChampionship").push({ Name: "Oliver Queen", ScorePerRound: [44, 46, 44] });
	* base.store("ArcheryChampionship").push({ Name: "Legolas", ScorePerRound: [50, 50, 48] });
	* // create a record set out of the records of the store
	* var recordSet = base.store("ArcheryChampionship").allRecords;
	* // filter the records: which archers have scored 48 points in the third round
	* recordSet.filter(function (rec) { return rec.ScorePerRound[2] == 48; }); // keeps only the records, where the score of the third round is equal 48
	* base.close();
	*/
	//# exports.RecordSet.prototype.filter = function (callback) { return Object.create(require('qminer').RecordSet.prototype); }; 
	JsDeclareFunction(filter);

	/**
	* Splits the record set into smaller record sets.
	* @param {function} callback - The splitter function. It takes two parameters (records) and returns a boolean object.
	* @returns {Array.<module:qm.RecordSet>} An array containing the smaller record sets. The records are split according the callback function.
	* @example
	* // import qm module
	* var qm = require("qminer");
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "SocialGames",
	*        fields: [
	*            { name: "Title", type: "string" },
	*            { name: "Type", type: "string" },
	*            { name: "MinPlayers", type: "int" },
	*            { name: "MaxPlayers", type: "int" }
	*        ]
	*    }]
	* });
	* // set new records in the store
	* base.store("SocialGames").push({ Title: "DungeonsAndDragons", Type: "Role-Playing", MinPlayers: 5, MaxPlayers: 5 });
	* base.store("SocialGames").push({ Title: "Dobble", Type: "Card", MinPlayers: 2, MaxPlayers: 8 });
	* base.store("SocialGames").push({ Title: "Settlers of Catan", Type: "Board", MinPlayers: 3, MaxPlayers: 4 });
	* base.store("SocialGames").push({ Title: "Munchkin", Type: "Card", MinPlayers: 3, MaxPlayers: 6 });
	* // create a record set out of the records of the store
	* var recordSet = base.store("SocialGames").allRecords;
	* // sort the records by MinPlayers in ascending order
	* recordSet.sortByField("MinPlayers", true);
	* // split the record set by the minimum number of players
	* // returns an array containing three record sets: the first containing the "DungeonsAndDragons" record,
	* // the second containing the "Settlers of Catan" and "Munchkin" records and the third containing the 
	* // "Dobble" record
	* var arr = recordSet.split(function (rec, rec2) { return rec.MinPlayers < rec2.MinPlayers; });
	* base.close();
	*/
	//# exports.RecordSet.prototype.split = function (callback) {return [Object.create(require('qminer').RecordSet.prototype)]; };
	JsDeclareFunction(split);

	/**
	* Deletes the records, that are also in the other record set.
	* @param {module:qm.RecordSet} rs - The other record set.
	* @returns {module:qm.RecordSet} Self. Contains only the records, that are not in rs.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "BookWriters",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Genre", type: "string" },
	*            { name: "Books", type: "string_v" }
	*        ]
	*    }]
	* });
	* // set new records in the store
	* base.store("BookWriters").push({ Name: "Terry Pratchett", Genre: "Fantasy", Books: ["The Colour of Magic", "Going Postal", "Mort", "Guards! Guards!"] });
	* base.store("BookWriters").push({ Name: "Douglas Adams", Genre: "Sci-fi", Books: ["The Hitchhiker's Guide to the Galaxy", "So Long, and Thanks for All the Fish"] });
	* base.store("BookWriters").push({ Name: "Fyodor Dostoyevsky", Genre: "Drama", Books: ["Crime and Punishment", "Demons"] });
	* base.store("BookWriters").push({ Name: "J.R.R. Tolkien", Genre: "Fantasy", Books: ["The Hobbit", "The Two Towers", "The Silmarillion" ] });
	* base.store("BookWriters").push({ Name: "George R.R. Martin", Genre: "Fantasy", Books: ["A Game of Thrones", "A Feast of Crows"] });
	* base.store("BookWriters").push({ Name: "J. K. Rowling", Genre: "Fantasy", Books: ["Harry Potter and the Philosopher's Stone"] });
	* base.store("BookWriters").push({ Name: "Ivan Cankar", Genre: "Drama", Books: ["On the Hill", "The King of Betajnova", "The Serfs"] });
	* // create one record set containing all records of store
	* var recordSet = base.store("BookWriters").allRecords;
	* // create one record set containing the records with genre "Fantasy"
	* var fantasy = base.store("BookWriters").allRecords.filterByField("Genre", "Fantasy");
	* // delete the records in recordSet, that are also in fantasy
	* recordSet.deleteRecords(fantasy); // returns self, containing only three records: "Douglas Adams", "Fyodor Dostoyevsky" and "Ivan Cankar"
	* base.close();
	*/
	//# exports.RecordSet.prototype.deleteRecords = function (rs) { return Object.create(require('qminer').RecordSet.prototype); }; 
	JsDeclareFunction(deleteRecords);

	/**
	* Returns the record set as a JSON.
	* @returns {Object} The record set as a JSON.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Musicians",
	*        fields: [
	*            { name: "Name", type: "string", primary: true },
	*            { name: "DateOfBirth", type: "datetime" },
	*            { name: "GreatestHits", type: "string_v" }
	*        ]
	*    }]
	* });
	* // create some records
	* base.store("Musicians").push({ Name: "Jimmy Page", DateOfBirth:  "1944-01-09T00:00:00", GreatestHits: ["Stairway to Heaven", "Whole Lotta Love"] });
	* base.store("Musicians").push({ Name: "Beyonce", DateOfBirth: "1981-09-04T00:00:00", GreatestHits: ["Single Ladies (Put a Ring on It)"] });
	* // create a record set out of the records in the "Musicians" store
	* var recordSet = base.store("Musicians").allRecords;
	* // create a JSON object out of the record set
	* var json = recordSet.toJSON();
	* base.close();
	*/
	//# exports.RecordSet.prototype.toJSON = function () {};
	JsDeclareFunction(toJSON);

	/**
	* Executes a function on each record in record set.
	* @param {function} callback - Function to be executed. It takes two parameters:
	* <br>rec - The current record.
	* <br>[idx] - The index of the current record.
	* @returns {module:qm.RecordSet} Self.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "People",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Gender", type: "string" }
	*        ]
	*    }]
	* });
	* // put some records in the store
	* base.store("People").push({ Name: "Eric Sugar", Gender: "Male" });
	* base.store("People").push({ Name: "Jane Tokyo", Gender: "Female" });
	* base.store("People").push({ Name: "Mister Tea", Gender: "Male" });
	* // create a record set out of the records of the store
	* var recordSet = base.store("People").allRecords;
	* // change the Name of all records into "Anonymous"
	* recordSet.each(function (rec) { rec.Name = "Anonymous"; }); // returns self, all record's Name are "Anonymous"
	* base.close();
	*/
	//# exports.RecordSet.prototype.each = function (callback) { return Object.create(require('qminer').RecordSet.prototype); }
	JsDeclareFunction(each);

	/**
	* Creates an array of function outputs created from the records in record set.
	* @param {function} callback - Function that generates the array. It takes two parameters:
	* <br>rec - The current record.
	* <br>[idx] - The index of the current record.
	* @returns {Array<Object>} The array created by the callback function.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "People",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Gender", type: "string" }
	*        ]
	*    }]
	* });
	* // put some records in the store
	* base.store("People").push({ Name: "Eric Sugar", Gender: "Male" });
	* base.store("People").push({ Name: "Jane Tokyo", Gender: "Female" });
	* base.store("People").push({ Name: "Mister Tea", Gender: "Male" });
	* // create a record set out of the records of the store
	* var recordSet = base.store("People").allRecords;
	* // make an array of record Names
	* var arr = recordSet.map(function (rec) { return rec.Name; }); // returns an array: ["Eric Sugar", "Jane Tokyo", "Mister Tea"]
	* base.close();
	*/
	//# exports.RecordSet.prototype.map = function (callback) { return [Object];  }
	JsDeclareFunction(map);

	/**
	* Creates the set intersection of two record sets.
	* @param {module:qm.RecordSet} rs - The other record set.
	* @returns {module:qm.RecordSet} The intersection of the two record sets.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base with one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Movies",
	*        fields: [
	*            { name: "Title", type: "string" },
	*            { name: "Length", type: "int" },
	*            { name: "Director", type: "string" }
	*        ]
	*    }]
	* });
	* // put some records in the store
	* base.store("Movies").push({ Title: "The Nightmare Before Christmas", Length: 76, Director: "Henry Selick" });
	* base.store("Movies").push({ Title: "Jurassic Part", Length: 127, Director: "Steven Spielberg" });
	* base.store("Movies").push({ Title: "The Avengers", Length: 143, Director: "Joss Whedon" });
	* base.store("Movies").push({ Title: "The Clockwork Orange", Length: 136, Director: "Stanley Kubrick" });
	* base.store("Movies").push({ Title: "Full Metal Jacket", Length: 116, Director: "Stanely Kubrick" });
	* // create a record set out of the records in store, where length of the movie is greater than 110
	* var greaterSet = base.store("Movies").allRecords.filterByField("Length", 110, 150);
	* // create a record set out of the records in store, where the length of the movie is lesser than 130
	* var lesserSet = base.store("Movies").allRecords.filterByField("Length", 0, 130);
	* // get the intersection of greaterSet and lesserSet
	* var intersection = greaterSet.setIntersect(lesserSet); // returns a record set, containing the movies with lengths between 110 and 130
	* base.close();
	*/
	//# exports.RecordSet.prototype.setIntersect = function (rs) { return Object.create(require('qminer').RecordSet.prototype); };
	JsDeclareFunction(setIntersect);

	/**
	* Creates the set union of two record sets.
	* @param {module:qm.RecordSet} rs - The other record set.
	* @returns {module:qm.RecordSet} The union of the two record sets.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "TVSeries",
	*        fields: [
	*            { name: "Title", type: "string", "primary": true },
	*            { name: "NumberOfEpisodes", type: "int" }
	*        ]
	*    }]
	* });
	* // add some records in the store
	* base.store("TVSeries").push({ Title: "Archer", NumberOfEpisodes: 75 });
	* base.store("TVSeries").push({ Title: "The Simpsons", NumberOfEpisodes: 574 });
	* base.store("TVSeries").push({ Title: "New Girl", NumberOfEpisodes: 94 });
	* base.store("TVSeries").push({ Title: "Rick and Morty", NumberOfEpisodes: 11 });
	* base.store("TVSeries").push({ Title: "Game of Thrones", NumberOfEpisodes: 47 });
	* // create a record set out of the records in store, where the number of episodes is lesser than 47
	* var lesserSet = base.store("TVSeries").allRecords.filterByField("NumberOfEpisodes", 0, 47);
	* // create a record set out of the records in store, where the number of episodes is greater than 100
	* var greaterSet = base.store("TVSeries").allRecords.filterByField("NumberOfEpisodes", 100, 600);
	* // get the union of lesserSet and greaterSet
	* var union = lesserSet.setUnion(greaterSet); // returns a record set, which is the union of the two record sets
	* base.close();
	*/
	//# exports.RecordSet.prototype.setUnion = function (rs) { return Object.create(require('qminer').RecordSet.prototype); };
	JsDeclareFunction(setUnion);

	/**
	* Creates the set difference between two record sets.
	* @param {module:qm.RecordSet} rs - The other record set.
	* @returns {module:qm.RecordSet} The difference between the two record sets.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "BookWriters",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Genre", type: "string" },
	*            { name: "Books", type: "string_v" }
	*        ]
	*    }]
	* });
	* // set new records in the store
	* base.store("BookWriters").push({ Name: "Terry Pratchett", Genre: "Fantasy", Books: ["The Colour of Magic", "Going Postal", "Mort", "Guards! Guards!"] });
	* base.store("BookWriters").push({ Name: "Douglas Adams", Genre: "Sci-fi", Books: ["The Hitchhiker's Guide to the Galaxy", "So Long, and Thanks for All the Fish"] });
	* base.store("BookWriters").push({ Name: "Fyodor Dostoyevsky", Genre: "Drama", Books: ["Crime and Punishment", "Demons"] });
	* base.store("BookWriters").push({ Name: "J.R.R. Tolkien", Genre: "Fantasy", Books: ["The Hobbit", "The Two Towers", "The Silmarillion" ] });
	* base.store("BookWriters").push({ Name: "George R.R. Martin", Genre: "Fantasy", Books: ["A Game of Thrones", "A Feast of Crows"] });
	* base.store("BookWriters").push({ Name: "J. K. Rowling", Genre: "Fantasy", Books: ["Harry Potter and the Philosopher's Stone"] });
	* base.store("BookWriters").push({ Name: "Ivan Cankar", Genre: "Drama", Books: ["On the Hill", "The King of Betajnova", "The Serfs"] });
	* // create one record set containing all records of store
	* var recordSet = base.store("BookWriters").allRecords;
	* // create one record set containing the records with genre "Fantasy"
	* var fantasy = base.store("BookWriters").allRecords.filterByField("Genre", "Fantasy");
	* // create a new record set containing the difference of recordSet and fantasy
	* var difference = recordSet.setDiff(fantasy); // returns a record set, containing the records of Douglas Adams, Fyodor Dostoyevsky and Ivan Cankar
	* base.close();
	*/
	//# exports.RecordSet.prototype.setDiff = function (rs) { return Object.create(require('qminer').RecordSet.prototype); }; 
	JsDeclareFunction(setDiff);

	/**
	* Creates a vector containing the field values of records.
	* @param {string} fieldName - The field from which to take the values. It's type must be one-dimensional, e.g. float, int, string,...
	* @returns {module:la.Vector} The vector containing the field values of records. The type it contains is dependant of the field type.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "TVSeries",
	*        fields: [
	*            { name: "Title", type: "string", "primary": true },
	*            { name: "NumberOfEpisodes", type: "int" }
	*        ]
	*    }]
	* });
	* // add some records in the store
	* base.store("TVSeries").push({ Title: "Archer", NumberOfEpisodes: 75 });
	* base.store("TVSeries").push({ Title: "The Simpsons", NumberOfEpisodes: 574 });
	* base.store("TVSeries").push({ Title: "New Girl", NumberOfEpisodes: 94 });
	* base.store("TVSeries").push({ Title: "Rick and Morty", NumberOfEpisodes: 11 });
	* base.store("TVSeries").push({ Title: "Game of Thrones", NumberOfEpisodes: 47 });
	* // create a record set of the records of store
	* var recordSet = base.store("TVSeries").allRecords;
	* // create a vector containing the number of episodes for each series
	* // the vector will look like [75, 574, 94, 11, 47]
	* var vector = recordSet.getVector("NumberOfEpisodes");
	* base.close();
	*/
	//# exports.RecordSet.prototype.getVector = function (fieldName) { return Object.create(require('qminer').la.Vector.prototype); }; 
	JsDeclareFunction(getVector);

	/**
	* Creates a vector containing the field values of records.
	* @param {string} fieldName - The field from which to take the values. It's type must be numeric, e.g. float, int, float_v, num_sp_v,...
	* @returns {(module:la.Matrix|module:la.SparseMatrix)} The matrix containing the field values of records.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "ArcheryChampionship",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "ScorePerRound", type: "float_v" }
	*        ]
	*    }]
	* });
	* // set new records in the store
	* base.store("ArcheryChampionship").push({ Name: "Robin Hood", ScorePerRound: [50, 48, 48] });
	* base.store("ArcheryChampionship").push({ Name: "Oliver Queen", ScorePerRound: [44, 46, 44] });
	* base.store("ArcheryChampionship").push({ Name: "Legolas", ScorePerRound: [50, 50, 48] });
	* // create a record set of the records in store
	* var recordSet = base.store("ArcheryChampionship").allRecords;
	* // create a matrix from the "ScorePerRound" field
	* // the i-th column of the matrix is the data of the i-th record in record set
	* // the matrix will look like
	* // 50  44  50
	* // 48  46  50
	* // 48  44  48
	* var matrix = recordSet.getMatrix("ScorePerRound");
	* base.close();
	*/
	//# exports.RecordSet.prototype.getMatrix = function (fieldName) { return Object.create(require('qminer').la.Matrix.prototype); };
	JsDeclareFunction(getMatrix);
	
	/**
	* Returns the store, where the records in the record set are stored.
	*/
	//# exports.RecordSet.prototype.store = Object.create(require('qminer').Store.prototype);
	JsDeclareFunction(store);

	/**
	* Returns the number of records in record set.
	*/
	//# exports.RecordSet.prototype.length = 0;
	JsDeclareProperty(length);

	/**
	* Checks if the record set is empty. If the record set is empty, then it returns true. Otherwise, it returns false.
	*/
	//# exports.RecordSet.prototype.empty = true;
	JsDeclareProperty(empty);

	/**
	* Checks if the record set is weighted. If the record set is weighted, then it returns true. Otherwise, it returns false.
	*/
	//# exports.RecordSet.prototype.weighted = true;
	JsDeclareProperty(weighted);

	//!- `rec = rs[n]` -- return n-th record from the record set
	JsDeclIndexedProperty(indexId);
};

///////////////////////////////
// NodeJs QMiner Store Iterator
/**
* Store Iterators allows you to iterate through the records in the store.
* <b>Factory pattern</b>: this class cannot be construced using the new keyword. It is constructed by calling
* a specific method or attribute, e.g. calling {@link module:qm.Store#forwardIter} to construct the Iterator.
* @class
* @example
* // import qm module
* qm = require('qminer');
* // create a new base with a simple store
* var base = new qm.Base({ mode: "createClean" });
* base.createStore({
*     name: "People",
*     fields: [
*         { name: "Name", type: "string" },
*         { name: "Gender", type: "string" }
*     ]
* });
* // add new records to the store
* base.store("People").push({ Name: "Geronimo", Gender: "Male" });
* base.store("People").push({ Name: "Pochahontas", Gender: "Female" });
* base.store("People").push({ Name: "John Rolfe", Gender: "Male" });
* base.store("People").push({ Name: "John Smith", Gender: "Male"});
* // factory based construction with forwardIter
* var iter = base.store("People").forwardIter;
* base.close();
*/
//# exports.Iterator = function () { return Object.create(require('qminer').Iterator.prototype); };

class TNodeJsStoreIter: public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	// Node framework
	static v8::Persistent<v8::Function> Constructor;
public:
	// Node framework 
	static void Init(v8::Handle<v8::Object> exports);
	static const TStr GetClassId() { return "StoreIter"; }

	// C++ wrapped object
	TWPt<TQm::TStore> Store;
	TQm::PStoreIter Iter;	
	TNodeJsRec* JsRec;
	// placeholder for last object
	v8::Persistent<v8::Object> RecObj;
	// Object that knows if Base is valid
	PNodeJsBaseWatcher Watcher;
	

	// C++ constructors
	TNodeJsStoreIter(const TWPt<TQm::TStore>& _Store, const TQm::PStoreIter& _Iter, PNodeJsBaseWatcher& _Watcher) : Store(_Store), Iter(_Iter), JsRec(nullptr), Watcher(_Watcher) {}
	
public:
	
    // delete placeholder
    ~TNodeJsStoreIter() { RecObj.Reset(); }
	
	/**
	* Moves to the next record.
	* @returns {boolean} 
	* <br>1. True, if the iteration successfully moves to the next record.
	* <br>2. False, if there is no record left.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "TheWitcherSaga",
	*        fields: [
	*            { name: "Title", type: "string" },
	*            { name: "YearOfRelease", type: "int" },
	*            { name: "EnglishEdition", type: "bool" }
	*        ]
	*    }]
	* });
	* // put some records in the store
	* base.store("TheWitcherSaga").push({ Title: "Blood of Elves", YearOfRelease: 1994, EnglishEdition: true });
	* base.store("TheWitcherSaga").push({ Title: "Time of Contempt", YearOfRelease: 1995, EnglishEdition: true });
	* base.store("TheWitcherSaga").push({ Title: "Baptism of Fire", YearOfRelease: 1996, EnglishEdition: true });
	* base.store("TheWitcherSaga").push({ Title: "The Swallow's Tower", YearOfRelease: 1997, EnglishEdition: false });
	* base.store("TheWitcherSaga").push({ Title: "Lady of the Lake", YearOfRelease: 1999, EnglishEdition: false });
	* base.store("TheWitcherSaga").push({ Title: "Season of Storms", YearOfRelease: 2013, EnglishEdition: false });
	* // create an iterator for the store
	* var iter = base.store("TheWitcherSaga").forwardIter;
	* // go to the first record in the store
	* iter.next(); // returns true
	* base.close();
	*/
	//# exports.Iterator.prototype.next = function () { return true; };
	JsDeclareFunction(next);

	/**
	* Gives the store of the iterator.
	*/
	//# exports.Iterator.prototype.store = Object.create(require('qminer').Store.prototype);
	JsDeclareFunction(store);

	/**
	* Gives the current record.
	*/
	//# exports.Iterator.prototype.record = Object.create(require('qminer').Record.prototype);
	JsDeclareFunction(record);
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
class TNodeJsIndexKey: public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
    // Node framework
    static v8::Persistent<v8::Function> Constructor;
public:
	// Node framework
    static void Init(v8::Handle<v8::Object> exports);
    static const TStr GetClassId() { return "IndexKey"; }
	// C++ wrapped object
	TWPt<TQm::TStore> Store;
	TQm::TIndexKey IndexKey;
	// Object that knows if Base is valid
	PNodeJsBaseWatcher Watcher;
    // C++ constructors
	TNodeJsIndexKey(const TWPt<TQm::TStore>& _Store, const TQm::TIndexKey& _IndexKey, PNodeJsBaseWatcher& _Watcher) :
		Store(_Store), IndexKey(_IndexKey), Watcher(_Watcher) { }
    
public:
	//!- `store = key.store` -- gets the key's store
	JsDeclareFunction(store);
	//!- `keyName = key.name` -- gets the key's name
	JsDeclareProperty(name);
	//!- `strArr = key.vocabulary` -- gets the array of words (as strings) in the vocabulary
	JsDeclareProperty(vocabulary);
	//!- `strArr = key.fq` -- gets the array of weights (as ints) in the vocabulary
	JsDeclareProperty(fq);
};


///////////////////////////////////////////////
/// Javscript Function Feature Extractor.
class TNodeJsFuncFtrExt : public TQm::TFtrExt {
private:
	// private constructor
	TNodeJsFuncFtrExt(const TWPt<TQm::TBase>& Base, const PJsonVal& ParamVal,
        const v8::Handle<v8::Function> _Fun, v8::Isolate* Isolate);
	~TNodeJsFuncFtrExt() { Fun.Reset(); }
public:
	// public smart pointer
	static TQm::PFtrExt NewFtrExt(const TWPt<TQm::TBase>& Base, const PJsonVal& ParamVal,
        const v8::Handle<v8::Function>& Fun, v8::Isolate* Isolate);
private:
	// Core part
	TInt Dim;
	TStr Name;
	// callback
	v8::Persistent<v8::Function> Fun;

	double ExecuteFunc(const TQm::TRec& FtrRec) const;
	void ExecuteFuncVec(const TQm::TRec& FtrRec, TFltV& Vec) const;
    // will throw exception (saving, loading not supported)
	TNodeJsFuncFtrExt(const TWPt<TQm::TBase>& Base, const PJsonVal& ParamVal);
    // will throw exception (saving, loading not supported)
	TNodeJsFuncFtrExt(const TWPt<TQm::TBase>& Base, TSIn& SIn);
public:
    // will throw exception (saving, loading not supported)
	static TQm::PFtrExt New(const TWPt<TQm::TBase>& Base, const PJsonVal& ParamVal);
    // will throw exception (saving, loading not supported)
	static TQm::PFtrExt Load(const TWPt<TQm::TBase>& Base, TSIn& SIn);
	void Save(TSOut& SOut) const;

	TStr GetNm() const { return Name; }
	int GetDim() const { return Dim; }
	TStr GetFtr(const int& FtrN) const { return TStr::Fmt("%s[%d]", GetNm().CStr(), FtrN) ; }

	void Clr() { };
	bool Update(const TQm::TRec& Rec) { return false; }
	void AddSpV(const TQm::TRec& Rec, TIntFltKdV& SpV, int& Offset) const;
	void AddFullV(const TQm::TRec& Rec, TFltV& FullV, int& Offset) const;
	void InvFullV(const TFltV& FullV, int& Offset, TFltV& InvV) const {
		throw TExcept::New("Not implemented yet!", "TJsFuncFtrExt::InvFullV"); }
	double GetVal(const double& InVal) const { throw TExcept::New("Not implemented!"); }

	// flat feature extraction
	void ExtractFltV(const TQm::TRec& FtrRec, TFltV& FltV) const;
	// feature extractor type name
	static TStr GetType() { return "jsfunc"; }
};

///////////////////////////////
// NodeJs QMiner Feature Space

/**
* Feature extractor types.
* @typedef {Object} FeatureExtractors
* @property {module:qm~FeatureExtractorConstant} constant - The constant type.
* @property {module:qm~FeatureExtractorRandom} random - The random type.
* @property {module:qm~FeatureExtractorNumeric} numeric - The numeric type.
* @property {module:qm~FeatureExtractorCategorical} categorical - The categorical type.
* @property {module:qm~FeatureExtractorMultinomial} multinomial - The multinomial type.
* @property {module:qm~FeatureExtractorText} text - The text type.
* @property {module:qm~FeatureExtractorJoin} join - The join type.
* @property {module:qm~FeatureExtractorPair} pair - The pair type.
* @property {module:qm~FeatureExtractorJsfunc} jsfunc - The jsfunc type.
* @property {module:qm~FeatureExtractorDateWindow} dateWindow - The date window type.
* @property {module:qm~FeatureExtractorSparseVector} sparseVector - The sparse vector type.
*
*/

/**
* @typedef {Object} FeatureExtractorConstant
* The feature extractor of type 'contant'.
* @property {string} FeatureExtractorConstant.type - The type of the extractor. It must be equal <b>'constant'</b>.
* @property {number} [FeatureExtractorConstant.const = 1.0] - A constant number.
* @property {module:qm~FeatureSource} FeatureExtractorConstant.source - The source of the extractor.
* @example
* var qm = require('qminer');
* // create a simple base, where each record contains only a persons name
* var base = new qm.Base({
*   mode: 'createClean',
*   schema: [{
*      name: "Person",
*      fields: [{ name: "Name", type: "string" }]
*   }]
* });
* // create a feature space containing the constant extractor, where the constant is equal 5
* var ftr = new qm.FeatureSpace(base, { type: "constant", source: "Person", const: 5 });
* base.close();
*/

/**
* @typedef {Object} FeatureExtractorRandom
* The feature extractor of type 'random'.
* @property {string} FeatureExtractorRandom.type - The type of the extractor. It must be equal <b>'random'</b>.
* @property {number} [FeatureExtractorRandom.seed = 0] - The seed number used to construct the random number.
* @property {module:qm~FeatureSource} FeatureExtractorRandom.source - The source of the extractor.
* @example
* var qm = require('qminer');
* // create a simple base, where each record contains only a persons name
* var base = new qm.Base({
*   mode: 'createClean',
*   schema: [{
*      name: "Person",
*      fields: [{ name: "Name", type: "string" }]
*   }]
* });
* // create a feature space containing the random extractor
* var ftr = new qm.FeatureSpace(base, { type: "random", source: "Person" });
* base.close();
*/

/**
* @typedef {Object} FeatureExtractorNumeric
* The feature extractor of type 'numeric'.
* @property {string} FeatureExtractorNumeric.type - The type of the extractor. It must be equal <b>'numeric'</b>.
* @property {boolean} [FeatureExtractorNumeric.normalize = 'false'] - Normalize values between 0.0 and 1.0.
* @property {number} [FeatureExtractorNumeric.min] - The minimal value used to form the normalization.
* @property {number} [FeatureExtractorNumeric.max] - The maximal value used to form the normalization.
* @property {string} FeatureExtractorNumeric.field - The name of the field from which to take the value.
* @property {module:qm~FeatureSource} FeatureExtractorNumeric.source - The source of the extractor.
* @example
* var qm = require('qminer');
* // create a simple base, where each record contains the student name and it's grade
* var base = new qm.Base({
*    mode: 'createClean',
*    schema: [{
*       name: "Class",
*       fields: [
*          { name: "Name", type: "string" },
*          { name: "Grade", type: "int" }
*       ]
*    }]
* });
* // create a feature space containing the numeric extractor, where the values are
* // normalized, the values are taken from the field "Grade"
* var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "Class", normalize: true, field: "Grade" });
* base.close();
*/

/**
* @typedef {Object} FeatureExtractorSparseVector
* The feature extractor of type 'num_sp_v'.
* @property {string} FeatureExtractorSparseVector.type - The type of the extractor. It must be equal <b>'num_sp_v'</b>.
* @property {number} [FeatureExtractorSparseVector.dimension = 0] - Dimensionality of sparse vectors.
* @property {boolean} [FeatureExtractorSparseVector.normalize = false] - Normalize vectors to L2 norm of 1.0.
* @property {string} FeatureExtractorSparseVector.field - The name of the field from which to take the value.
* @property {module:qm~FeatureSource} FeatureExtractorSparseVector.source - The source of the extractor.
* @example
* var qm = require('qminer');
* // create a simple base, where each record contains the student name and it's grade
* var base = new qm.Base({
*    mode: 'createClean',
*    schema: [{
*       "name": "Class",
*       "fields": [
*          { "name": "Name", "type": "string" },
*          { "name": "Features", "type": "num_sp_v" }
*       ]
*    }]
* });
* // create a feature space containing the numeric extractor, where the values are
* // normalized, the values are taken from the field "Grade"
* var ftr = new qm.FeatureSpace(base, { type: "num_sp_v", source: "Class", normalize: false, field: "Features" });
* base.close();
*/

/**
* @typedef {Object} FeatureExtractorCategorical
* The feature extractor of type 'categorical'.
* @property {string} FeatureExtractorCategorical.type - The type of the extractor. It must be equal <b>'categorical'</b>.
* @property {Array.<Object>} [FeatureExtractorCategorical.values] - A fixed set of values, which form a fixed feature set. No dimensionality changes if new values are seen in the upgrades.
* @property {number} [FeatureExtractorCategorical.hashDimension] - A hashing code to set the fixed dimensionality. All values are hashed and divided modulo hasDimension to get the corresponding dimension.
* @property {string} FeatureExtractorCategorical.field - The name of the field form which to take the values.
* @property {module:qm~FeatureSource} FeatureExtractorCategorical.source - The source of the extractor.
* @example
* var qm = require('qminer');
* // create a simple base, where each record contains the student name and it's study group
* // here we know the student is part of only one study group
* var base = new qm.Base({
*    mode: 'createClean',
*    schema: [{
*       name: "Class",
*       fields: [
*          { name: "Name", type: "string" },
*          { name: "StudyGroup", type: "string" }
*       ]
*    }]
* });
* // create a feature space containing the categorical extractor, where the it's values
* // are taken from the field "StudyGroup": "A", "B", "C" and "D"
* var ftr = new qm.FeatureSpace(base, { type: "categorical", source: "Class", field: "StudyGroup", values: ["A", "B", "C", "D"] });
* base.close();
*/

/**
* @typedef {Object} FeatureExtractorMultinomial
* The feature extractor of type 'multinomial'.
* @property {string} FeatureExtractorMultinomial.type - The type of the extractor. It must be equal <b>'multinomial'</b>.
* @property {boolean} [FeatureExtractorMultinomial.normalize = 'false'] - Normalize the resulting vector of the extractor to have L2 norm 1.0.
* @property {Array.<Object>} [FeatureExtractorMultinomial.values] - A fixed set of values, which form a fixed feature set, no dimensionality changes if new values are seen in the updates. Cannot be used the same time as datetime.
* @property {number} [FeatureExtractorMultinomial.hashDimension] - A hashing code to set the fixed dimensionality. All values are hashed and divided modulo hashDimension to get the corresponding dimension.
* @property {Object} [FeatureExtractorMultinomial.datetime = false] - Same as 'values', only with predefined values which are extracted from date and time (month, day of month, day of week, time of day, hour).
* <br> This fixes the dimensionality of feature extractor at the start, making it not dimension as new dates are seen. Cannot be used the same time as values.
* @property {(string|Array.<String>)} FeatureExtractorMultinomial.field - The name of the field from which to take the key value.
* @property {(string|Array.<String>)} [FeatureExtractorMultinomial.valueField] - The name of the field from which to take the numeric value. When not provided, 1.0 is used as default numeric values for non-zero elements in the vector.
* @property {module:qm~FeatureSource} FeatureExtractorMultinomial.source - The source of the extractor.
* @example
* var qm = require('qminer');
* // create a simple base, where each record contains the student name and an array of study groups
* // here we know a student can be part of multiple study groups
* var base = new qm.Base({
*    mode: 'createClean',
*    schema: [{
*       name: "Class",
*       fields: [
*          { name: "Name", type: "string" },
*          { name: "StudyGroups", type: "string_v" }
*       ]
*    }]
* });
* // create a feature space containing the multinomial extractor, where the values are normalized,
* // and taken from the field "StudyGroup": "A", "B", "C", "D", "E", "F"
* var ftr = new qm.FeatureSpace(base, {
*              type: "multinomial", source: "Class", field: "StudyGroups", normalize: true, values: ["A", "B", "C", "D", "E", "F"]
*           });
* base.close();
*/

/**
* @typedef {Object} FeatureExtractorText
* The feature extractor of type 'text'.
* @property {string} FeatureExtractorText.type - The type of the extractor. It must be equal <b>'text'</b>.
* @property {boolean} [FeatureExtractorText.normalize = 'true'] - Normalize the resulting vector of the extractor to have L2 norm 1.0.
* @property {module:qm~FeatureWeight} [FeatureExtractorText.weight = 'tfidf'] - Type of weighting used for scoring terms.
* @property {number} [FeatureExtractorText.hashDimension] - A hashing code to set the fixed dimensionality. All values are hashed and divided modulo hashDimension to get the corresponding dimension.
* @property {string} FeatureExtractorText.field - The name of the field from which to take the value.
* @property {module:qm~FeatureTokenizer} FeatureExtractorText.tokenizer - The settings for extraction of text.
* @property {module:qm~FeatureMode} FeatureExtractorText.mode - How are multi-record cases combined into single vector.
* @property {module:qm~FeatureStream} FeatureExtractorText.stream - Details on forgetting old IDFs when running on stream.
* @property {module:qm~FeatureSource} FeatureExtractorText.source - The source of the extractor.
* @example
* var qm = require('qminer');
* // create a simple base, where each record contains the title of the article and it's content
* var base = new qm.Base({
*    mode: 'createClean',
*    schema: [{
*       name: "Articles",
*       fields: [
*          { name: "Title", type: "string" },
*          { name: "Text", type: "string" }
*       ]
*    }]
* });
* // create a feature spave containing the text (bag of words) extractor, where the values are normalized,
* // weighted with 'tfidf' and the tokenizer is of 'simple' type, it uses english stopwords.
* var ftr = new qm.FeatureSpace(base, {
*              type: "text", source: "Articles", field: "Text", normalize: true, weight: "tfidf",
*              tokenizer: { type: "simple", stopwords: "en"}
*           });
* base.close();
*/

/**
* @typedef {Object} FeatureExtractorJoin
* The feature extractor of type 'join'.
* @property {string} FeatureExtractorJoin.type - The type of the extractor. It must be equal <b>'join'</b>.
* @property {number} [FeatureExtractorJoin.bucketSize = 1] - The size of the bucket in which we group consecutive records.
* @property {module:qm~FeatureSource} FeatureExtractorJoin.source - The source of the extractor.
* @example
* // import qm module
* var qm = require('qminer');
*/

/**
* @typedef {Object} FeatureExtractorPair
* The feature extractor of type 'pair'.
* @property {string} FeatureExtractorPair.type - The type of the extractor. It must be equal <b>'pair'</b>.
* @property {module:qm~FeatureExtractors} FeatureExtractorPair.first - The first feature extractor.
* @property {module:qm~FeatureExtractors} FeatureExtractorPair.second - The second feature extractor.
* @property {module:qm~FeatureSource} FeatureExtractorPair.source - The source of the extractor.
* @example
* var qm = require('qminer');
*/

/**
* @typedef {Object} FeatureExtractorDateWindow
* The feature extractor of type 'dateWindow'.
* @property {string} FeatureExtractorDateWindow.type - The type of the extractor. It must be equal <b>'dateWindow'</b>.
* @property {string} [FeatureExtractorDateWindow.unit = 'day'] - How granular is the time window. The options are: 'day', 'week', 'month', 'year', '12hours', '6hours', '4hours', '2hours',
* 'hour', '30minutes', '15minutes', '10minutes', 'minute', 'second'.
* @property {number} [FeatureExtractorDateWindow.window = 1] - The size of the window.
* @property {boolean} [FeatureExtractorDateWindow.normalize = 'false'] - Normalize the resulting vector of the extractor to have L2 norm 1.0. //TODO
* @property {number} FeatureExtractorDateWindow.start - //TODO
* @property {number} FeatureExtractorDateWindow.end - //TODO
* @property {module:qm~FeatureSource} FeatureExtractorDateWindow.source - The source of the extractor.
* @example
* // import qm module
* var qm = require('qminer');
*/

/**
* @typedef {Object} FeatureExtractorJsfunc
* The feature extractor of type 'jsfunc'.
* @property {string} FeatureExtractorJsfunc.type - The type of the extractor. It must be equal <b>'jsfunc'</b>.
* @property {string} FeatureExtractorJsfunc.name - The feature's name.
* @property {function} FeatureExtractorJsfunc.fun - The javascript function callback. It should take a record as input and return a number or a dense vector.
* @property {number} [FeatureExtractorJsfunc.dim = 1] - The dimension of the feature extractor.
* @property {module:qm~FeatureSource} FeatureExtractorJsfunc.source - The source of the extractor.
* @example
* var qm = require('qminer');
* // create a simple base, where each record contains the name of the student and his study groups
* // each student is part of multiple study groups
* var base = new qm.Base({
*    mode: 'createClean',
*    schema: [{
*       name: "Class",
*       fields: [
*          { name: "Name", type: "string" },
*          { name: "StudyGroups", type: "string_v" }
*       ]
*    }]
* });
* // create a feature space containing the jsfunc extractor, where the function counts the number
* // of study groups each student is part of. The functions name is "NumberOFGroups", it's dimension
* // is 1 (returns only one value, not an array)
* var ftr = new qm.FeatureSpace(base, {
*              type: "jsfunc", source: "Class", name: "NumberOfGroups", dim: 1,
*              fun: function (rec) { return rec.StudyGroups.length; }
*           });
* base.close();
*/

/**
* From where are the input records taken.
* @typedef {Object} FeatureSource
* @property {string} FeatureSource.store - The store name.
*/

/**
* Type of weighting used for scoring terms.
* @readonly
* @enum {string}
*/
//# var FeatureWeight = {
//#		/** Sets 1 if term occurs, 0 otherwise. */
//#		none: 'none',
//#		/** Sets the term frequency in the document. */
//#		tf: 'tf',
//#		/** Sets the inverse document frequency in the document. */
//#		idf: 'idf',
//#		/** Sets the product of the tf and idf frequency. */
//#		tfidf: 'tfidf'
//# }

/**
* The settings for extraction of text.
* @typedef {Object} FeatureTokenizer
* @property {module:qm~FeatureTokenizerType} [FeatureTokenizer.type = 'simple'] - The type of the encoding text.
* @property {module:qm~FeatureTokenizerStopwords} [FeatureTokenizer.stopwords = 'en'] - The stopwords used for extraction.
* @property {module:qm~FeatureTokenizerStemmer} [FeatureTokenizer.stemmer = 'none'] - The stemmer used for extraction.
* @property {boolean} [FeatureTokenizer.uppercase = 'true'] - Changing all words to uppercase.
*/

/**
* The type of the encoding text.
* @readonly
* @enum {string}
*/
//# var FeatureTokenizerType = {
//#		/** The simple encoding. */
//#		simple: 'simple',
//#		/** The html encoding. */
//#		html: 'html',
//#		/** The unicode encoding. */
//#		unicode: 'unicode'
//# }

/**
* THe stopwords used for extraction.
* @readonly
* @enum {Object}
*/
//# var FeatureTokenizerStopwords = {
//#		/** The pre-defined stopword list (none). */
//#		none: 'none',
//#		/** The pre-defined stopword list (english). */
//#		en: 'en',
//#		/** The pre-defined stopword list (slovene). */
//#		si: 'si',
//#		/** The pre-defined stopword list (spanish). */
//#		es: 'es',
//#		/** The pre-defined stopword list (german). */
//#		de: 'de',
//#		/** An array of stopwords. The array must be given as a parameter instead of 'array'! */
//#		array: 'array'
//# }

/**
* The steemer used for extraction.
* @readonly
* @enum {Object}
*/
//# var FeatureTokenizerStemmer = {
//#		/** For using the porter stemmer. */
//#		boolean: 'true',
//#		/** For using the porter stemmer. */
//#		porter: 'porter',
//#		/** For using no stemmer. */
//#		none: 'none',
//# }

/**
* How are multi-record cases combined into a single vector. //TODO not implemented for join record cases (works only if the start store and the
* feature store are the same)
* @readonly
* @enum {string}
*/
//# var FeatureMode = {
//#		/** Multi-record cases are merged into one document. */
//#		concatenate: 'concatenate', 
//#		/** Treat each case as a separate document. */
//#		centroid: 'centroid', 
//#		/** Use the tokenizer option */
//#		tokenized : 'tokenized'
//# }

/**
* Details on forgetting old IDFs when running on stream.
* @readonly
* @enum {string}
*/
//# var FeatureStream = {
//#		/** (optional) Field name which is providing timestamp, if missing system time is used. */
//#		field: 'field',
//#		/** Forgetting factor, by which the old IDFs are multiplied after each iteration. */
//#		factor: 'factor',
//#		/** The time between iterations when the factor is applied, standard JSon time format is used to specify the interval duration. */
//#		interval: 'interval'
//# }

/////////////////////////////
// Feature Space Desc

/**
* Feature Space
* @classdesc Represents the feature space. It contains any of the {@link module:qm~FeatureExtractors}.
* @class
* @param {module:qm.Base} base - The base where the features are extracted from.
* @param {(Array.<Object> | module:fs.FIn)} param - Array with definiton of extractors or input stream.
* @example
* // import qm module
* var qm = require('qminer');
* // construct a base with the store
* var base = new qm.Base({
*   mode: "createClean",
*   schema: {
*     name: "FtrSpace",
*     fields: [
*       { name: "Value", type: "float" },
*       { name: "Category", type: "string" },
*       { name: "Categories", type: "string_v" },
*     ],
*     joins: [],
*     keys: []
*   }
* });
* // populate the store
* Store = base.store("FtrSpace");
* Store.push({ Value: 1.0, Category: "a", Categories: ["a", "q"] });
* Store.push({ Value: 1.1, Category: "b", Categories: ["b", "w"] });
* Store.push({ Value: 1.2, Category: "c", Categories: ["c", "e"] });
* Store.push({ Value: 1.3, Category: "a", Categories: ["a", "q"] });
* // create a feature space 
* var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpace", field: "Value" });
* base.close();
*/
//# exports.FeatureSpace = function (base, extractors) { return Object.create(require('qminer').FeatureSpace.prototype); };

class TNodeJsFtrSpace : public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	static v8::Persistent<v8::Function> Constructor;
public:
	// Node framework
	static void Init(v8::Handle<v8::Object> exports);
	static const TStr GetClassId() { return "FeatureSpace"; }

	TQm::PFtrSpace FtrSpace;
	TNodeJsFtrSpace(const TQm::PFtrSpace& FtrSpace);
	TNodeJsFtrSpace(const TWPt<TQm::TBase> Base, TSIn& SIn);
	
	TQm::PFtrSpace GetFtrSpace() { return FtrSpace; }
	static TNodeJsFtrSpace* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

private:
	class TUpdateRecsTask: public TNodeTask {
	private:
		TNodeJsFtrSpace* JsFtrSpace;
		TNodeJsRecByValV* JsRecV;

	public:
		TUpdateRecsTask(const v8::FunctionCallbackInfo<v8::Value>& Args);

		v8::Handle<v8::Function> GetCallback(const v8::FunctionCallbackInfo<v8::Value>& Args);
		void Run();
	};

    class TExtractMatrixTask: public TNodeTask {
    private:
    	TNodeJsFtrSpace* JsFtrSpace;
    	TNodeJsRecByValV* JsRecV;
    	TNodeJsFltVV* JsFtrVV;

    public:
    	TExtractMatrixTask(const v8::FunctionCallbackInfo<v8::Value>& Args);

    	v8::Handle<v8::Function> GetCallback(const v8::FunctionCallbackInfo<v8::Value>& Args);
    	void Run();
    	v8::Local<v8::Value> WrapResult();
    };

public:

    /**
	* Returns the dimension of the feature space.
	*/
	//# exports.FeatureSpace.prototype.dim = 0;
    JsDeclareProperty(dim);
    
	/**
	* Returns an array of the dimensions of each feature extractor in the feature space.
	*/
	//# exports.FeatureSpace.prototype.dims = [0];
    JsDeclareProperty(dims);

	/**
	* Serialize the feature space to an output stream.
	* @param {module:fs.FOut} fout - The output stream.
	* @returns {module:fs.FOut} The output stream.
	*/
	//# exports.FeatureSpace.prototype.save = function (fout) { return Object.create(require('qminer').fs.FOut.prototype); };
    JsDeclareFunction(save);

	/**
	* Clears the feature space.
	* @returns {module:qm.FeatureSpace} Self. 
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Runners",
	*        fields: [
	*            { name: "ID", type: "int", primary: true },
	*            { name: "Name", type: "string" },
	*            { name: "BestTime", type: "float" }
	*        ]
	*    }]
	* });
	* // put some records in the "Runners" store
	* base.store("Runners").push({ ID: 110020, Name: "Eric Ericsson", BestTime: 134.33 });
	* base.store("Runners").push({ ID: 123307, Name: "Fred Friedrich", BestTime: 101.11 });
	* base.store("Runners").push({ ID: 767201, Name: "Appel Banana", BestTime: 1034.56 });
	* // create a feature space
	* var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "Runners", field: "BestTime" });
	* // update the feature space
	* ftr.updateRecords(base.store("Runners").allRecords);
	* // clear the feature space (return the feature space to it's default values)
	* ftr.clear();
	* base.close();
	*/
	//# exports.FeatureSpace.prototype.clear = function () { return Object.create(require('qminer').FeatureSpace.prototype); };
	JsDeclareFunction(clear);

	/**
	* Adds a new feature extractor to the feature space.
	* @param {Object} obj - The added feature extractor. It must be given as a JSON object.
	* @returns {module:qm.FeatureSpace} Self.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "WeatherForcast",
	*        fields: [
	*            { name: "Weather", type: "string" },
	*            { name: "Date", type: "datetime" },
	*            { name: "TemperatureDegrees", type: "int" }
	*        ]
	*    }]
	* });
	* // put some records in the "WeatherForecast" store
	* base.store("WeatherForcast").push({ Weather: "Partly Cloudy", Date: "2015-05-27T11:00:00", TemperatureDegrees: 19 });
	* base.store("WeatherForcast").push({ Weather: "Partly Cloudy", Date: "2015-05-28T11:00:00", TemperatureDegrees: 22 });
	* base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-05-29T11:00:00", TemperatureDegrees: 25 });
	* base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-05-30T11:00:00", TemperatureDegrees: 25 });
	* base.store("WeatherForcast").push({ Weather: "Scattered Showers", Date: "2015-05-31T11:00:00", TemperatureDegrees: 24 });
	* base.store("WeatherForcast").push({ Weather: "Mostly Cloudy", Date: "2015-06-01T11:00:00", TemperatureDegrees: 27 });
	* // create a feature space 
	* var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "WeatherForcast", field: "TemperatureDegrees" });
	* // add a new feature extractor to the feature space
	* // it adds the new feature extractor to the pre-existing feature extractors in the feature space
	* ftr.addFeatureExtractor({ type: "text", source: "WeatherForcast", field: "Weather", normalize: true, weight: "tfidf" });      
	* base.close();
	*/
	//# exports.FeatureSpace.prototype.addFeatureExtractor = function (obj) { return Object.create(require('qminer').FeatureSpace.prototype); };
	JsDeclareFunction(addFeatureExtractor);

	/**
	* Updates the feature space definitions and extractors by adding one record.
	* <br> For text feature extractors, it can update it's vocabulary by taking into account the new text.
	* <br> For numeric feature extractors, it can update the minimal and maximal values used to form the normalization.
	* <br> For jsfunc feature extractors, it can update a parameter used in it's function.
	* <br> For dateWindow feature extractor, it can update the start and the end of the window period to form the normalization.
	* @param {module:qm.Record} rec - The record, which updates the feature space.
	* @returns {module:qm.FeatureSpace} Self.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base
	* var base = new qm.Base({
    *   mode: "createClean",
    *   schema: {
    *     name: "FtrSpace",
    *     fields: [
    *       { name: "Value", type: "float" },
    *       { name: "Category", type: "string" },
    *       { name: "Categories", type: "string_v" },
    *     ]
    *   }
    * });
    * // populate the store
    * Store = base.store("FtrSpace");
    * Store.push({ Value: 1.0, Category: "a", Categories: ["a", "q"] });
    * Store.push({ Value: 1.1, Category: "b", Categories: ["b", "w"] });
    * Store.push({ Value: 1.2, Category: "c", Categories: ["c", "e"] });
    * Store.push({ Value: 1.3, Category: "a", Categories: ["a", "q"] });
	* // create a new feature space
	* var ftr = new qm.FeatureSpace(base, [
	*   { type: "numeric", source: "FtrSpace", normalize: true, field: "Value" },
	*   { type: "categorical", source: "FtrSpace", field: "Category", values: ["a", "b", "c"] },
	*   { type: "multinomial", source: "FtrSpace", field: "Categories", normalize: true, values: ["a", "b", "c", "q", "w", "e"] }
	* ]);
	* // update the feature space with the first three record of the store
	* ftr.updateRecord(Store[0]);
	* ftr.updateRecord(Store[1]);
	* ftr.updateRecord(Store[2]);
	* // get the feature vectors of these records
	* ftr.extractVector(Store[0]); // returns the vector [0, 1, 0, 0, 1 / Math.sqrt(2), 0, 0, 1 / Math.sqrt(2), 0, 0]
	* ftr.extractVector(Store[1]); // returns the vector [1/2, 0, 1, 0, 0, 1 / Math.sqrt(2), 0, 0, 1 / Math.sqrt(2), 0]
	* ftr.extractVector(Store[2]); // returns the vector [1, 0, 0, 1, 0, 0, 1 / Math.sqrt(2), 0, 0, 1 / Math.sqrt(2)]
	* base.close();
	*/
	//# exports.FeatureSpace.prototype.updateRecord = function (rec) { return Object.create(require('qminer').FeatureSpace.prototype); };
	JsDeclareFunction(updateRecord);

	/**
	* Updates the feature space definitions and extractors by adding all the records of a record set.
	* <br> For text feature extractors, it can update it's vocabulary by taking into account the new text.
	* <br> For numeric feature extractors, it can update the minimal and maximal values used to form the normalization.
	* <br> For jsfunc feature extractors, it can update a parameter used in it's function.
	* <br> For dateWindow feature extractor, it can update the start and the end of the window period to form the normalization.
	* @param {module:qm.RecordSet} rs - The record set, which updates the feature space.
	* @returns {module:qm.FeatureSpace} Self.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base
	* var base = new qm.Base({
    *   mode: "createClean",
    *   schema: {
    *     name: "FtrSpace",
    *     fields: [
    *       { name: "Value", type: "float" },
    *       { name: "Category", type: "string" },
    *       { name: "Categories", type: "string_v" },
    *     ]
    *   }
    * });
    * // populate the store
	* Store = base.store("FtrSpace");
	* Store.push({ Value: 1.0, Category: "a", Categories: ["a", "q"] });
	* Store.push({ Value: 1.1, Category: "b", Categories: ["b", "w"] });
	* Store.push({ Value: 1.2, Category: "c", Categories: ["c", "e"] });
	* Store.push({ Value: 1.3, Category: "a", Categories: ["a", "q"] });
	* // create a new feature space
	* var ftr = new qm.FeatureSpace(base, [
	*     { type: "numeric", source: "FtrSpace", normalize: true, field: "Value" },
	*     { type: "categorical", source: "FtrSpace", field: "Category", values: ["a", "b", "c"] },
	*     { type: "multinomial", source: "FtrSpace", field: "Categories", normalize: true, values: ["a", "b", "c", "q", "w", "e"] }
	* ]);
	* // update the feature space with the record set 
	* ftr.updateRecords(Store.allRecords);
	* // get the feature vectors of these records
	* ftr.extractVector(Store[0]); // returns the vector [0, 1, 0, 0, 1 / Math.sqrt(2), 0, 0, 1 / Math.sqrt(2), 0, 0]
	* ftr.extractVector(Store[1]); // returns the vector [1/3, 0, 1, 0, 0, 1 / Math.sqrt(2), 0, 0, 1 / Math.sqrt(2), 0]
	* ftr.extractVector(Store[2]); // returns the vector [2/3, 0, 0, 1, 0, 0, 1 / Math.sqrt(2), 0, 0, 1 / Math.sqrt(2)]
	* ftr.extractVector(Store[3]); // returns the vector [1, 1, 0, 0, 1 / Math.sqrt(2), 0, 0, 1 / Math.sqrt(2), 0, 0]
	* base.close();
	*/
	//# exports.FeatureSpace.prototype.updateRecords = function (rs) { return Object.create(require('qminer').FeatureSpace.prototype); };
	JsDeclareFunction(updateRecords);

	JsDeclareAsyncFunction(updateRecordsAsync, TUpdateRecsTask);

	/**
	* Creates a sparse feature vector from the given record.
	* @param {module:qm.Record} rec - The given record.
    * @param {number} [featureExtractorId] - when given, only use specified feature extractor.
	* @returns {module:la.SparseVector} The sparse feature vector gained from rec.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a base containing the store Class. Let the Name field be the primary field. 
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Class",
	*        fields: [
	*            { name: "Name", type: "string", primary: true },
	*            { name: "StudyGroup", type: "string" }
	*        ]
	*    }]
	* });
	* // add some records to the store
	* base.store("Class").push({ Name: "Dean", StudyGroup: "A" });
	* base.store("Class").push({ Name: "Chang", StudyGroup: "D" });
	* base.store("Class").push({ Name: "Magnitude", StudyGroup: "C" });
	* base.store("Class").push({ Name: "Leonard", StudyGroup: "B" });
	* // create a new feature space
	* // the feature space is of dimensions [0, 4]; 4 is the dimension of the categorical feature extractor
	* // and 0 is the dimension of text feature extractor (the text feature extractor doesn't have any words,
	* // need to be updated for use).
	* var ftr = new qm.FeatureSpace(base, [
	*    { type: "text", source: "Class", field: "Name", normalize: false },
	*    { type: "categorical", source: "Class", field: "StudyGroup", values: ["A", "B", "C", "D"] } 
	* ]);
	* // get the sparse extractor vector for the first record in store
	* // the sparse vector will be [(0, 1)] - uses only the categorical feature extractor. There are no
	* // features in the text feature extractor.
	* var vec = ftr.extractSparseVector(base.store("Class")[0]);
	* base.close();
	*/
	//# exports.FeatureSpace.prototype.extractSparseVector = function (rec) { return Object.create(require('qminer').la.SparseVector.prototype); }
    JsDeclareFunction(extractSparseVector);

	/**
	* Creates a feature vector from the given record.
	* @param {module:qm.Record} rec - The given record.
    * @param {number} [featureExtractorId] - when given, only use specified feature extractor.
	* @returns {module:la.Vector} The feature vector gained from rec.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a base containing the store Class. Let the Name field be the primary field.
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Class",
	*        fields: [
	*            { name: "Name", type: "string", primary: true },
	*            { name: "StudyGroup", type: "string" }
	*        ]
	*    }]
	* });
	* // add some records to the store
	* base.store("Class").push({ Name: "Jeff", StudyGroup: "A" });
	* base.store("Class").push({ Name: "Britta", StudyGroup: "D" });
	* base.store("Class").push({ Name: "Abed", StudyGroup: "C" });
	* base.store("Class").push({ Name: "Annie", StudyGroup: "B" });
	* // create a new feature space
	* // the feature space is of dimensions [0, 4]; 4 is the dimension of the categorical feature extractor
	* // and 0 is the dimension of text feature extractor (the text feature extractor doesn't have any words,
	* // need to be updated for use).
	* var ftr = new qm.FeatureSpace(base, [
	*    { type: "text", source: "Class", field: "Name", normalize: false },
	*    { type: "categorical", source: "Class", field: "StudyGroup", values: ["A", "B", "C", "D"] }
	* ]);
	* // get the extractor vector for the first record in store
	* // the sparse vector will be [1, 0, 0, 0] - uses only the categorical feature extractor. There are no
	* // features in the text feature extractor.
	* var vec = ftr.extractVector(base.store("Class")[0]);
	* base.close();
	*/
	//# exports.FeatureSpace.prototype.extractVector = function (rec) { return Object.create(require('qminer').la.Vector.prototype); };
	JsDeclareFunction(extractVector);

	/**
	 * Extracts a single feature using the feature extractor at index ftrIdx.
	 *
	 * @param {Integer} ftrIdx - index of the feature extractor
	 * @param {Number} val - value to extract
	 */
	JsDeclareFunction(extractFeature);
    
	/**
	* Extracts the sparse feature vectors from the record set and returns them as columns of the sparse matrix.
	* @param {module:qm.RecordSet} rs - The given record set.
    * @param {number} [featureExtractorId] - when given, only use specified feature extractor
    * @returns {module:la.SparseMatrix} The sparse matrix, where the i-th column is the sparse feature vector of the i-th record in rs.
	* @example
	* // import qm module
	* var qm = require("qminer");
	* // create a base containing the store Class. Let the Name field be the primary field. 
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Class",
	*        fields: [
	*            { name: "Name", type: "string", primary: true },
	*            { name: "StudyGroups", type: "string_v" }
	*        ]
	*    }]
	* });
	* // add some records to the store
	* base.store("Class").push({ Name: "Dean", StudyGroups: ["A", "D"] });
	* base.store("Class").push({ Name: "Chang", StudyGroups: ["B", "D"] });
	* base.store("Class").push({ Name: "Magnitude", StudyGroups: ["B", "C"] });
	* base.store("Class").push({ Name: "Leonard", StudyGroups: ["A", "B"] });
	* // create a feature space containing the multinomial feature extractor
	* var ftr = new qm.FeatureSpace(base, { type: "multinomial", source: "Class", field: "StudyGroups", values: ["A", "B", "C", "D"] });
	* // create a sparse feature matrix out of the records of the store by using the feature space
	* // returns a sparse matrix equal to 
	* // [[(0, 1), (3, 1)], [(1, 1), (3, 1)], [(1, 1), (2, 1)], [(0, 1), (1, 1)]]
	* var sparseMatrix = ftr.extractSparseMatrix(base.store("Class").allRecords);
	* base.close();
	*/
	//# exports.FeatureSpace.prototype.extractSparseMatrix = function (rs) { return Object.create(require('qminer').la.SparseMatrix.prototype); };
	JsDeclareFunction(extractSparseMatrix);

	/**
	* Extracts the feature vectors from the recordset and returns them as columns of a dense matrix.
	* @param {module:qm.RecordSet} rs - The given record set.
    * @param {number} [featureExtractorId] - when given, only use specified feature extractor.

	* @returns {module:la.Matrix} The dense matrix, where the i-th column is the feature vector of the i-th record in rs.
	* @example
	* // import qm module
	* var qm = require("qminer");
	* // create a base containing the store Class. Let the Name field be the primary field.
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Class",
	*        fields: [
	*            { name: "Name", type: "string", primary: true },
	*            { name: "StudyGroups", type: "string_v" }
	*        ]
	*    }]
	* });
	* // add some records to the store
	* base.store("Class").push({ Name: "Dean", StudyGroups: ["A", "D"] });
	* base.store("Class").push({ Name: "Chang", StudyGroups: ["B", "D"] });
	* base.store("Class").push({ Name: "Magnitude", StudyGroups: ["B", "C"] });
	* base.store("Class").push({ Name: "Leonard", StudyGroups: ["A", "B"] });
	* // create a feature space containing the multinomial feature extractor
	* var ftr = new qm.FeatureSpace(base, { type: "multinomial", source: "Class", field: "StudyGroups", values: ["A", "B", "C", "D"] });
	* // create a feature matrix out of the records of the store by using the feature space
	* // returns a sparse matrix equal to
	* // 1  0  0  1
	* // 0  1  0  1
	* // 0  0  1  0
	* // 1  1  0  0
	* var matrix = ftr.extractMatrix(base.store("Class").allRecords);
	* base.close();
	*/
	//# exports.FeatureSpace.prototype.extractMatrix = function (rs) { return Object.create(require('qminer').la.Matrix.prototype); };
    JsDeclareFunction(extractMatrix);

    JsDeclareAsyncFunction(extractMatrixAsync, TExtractMatrixTask);

	/**
	* Gives the name of feature extractor at given position.
	* @param {number} idx - The index of the feature extractor in feature space (zero based).
	* @returns {String} The name of the feature extractor at position idx.
	* @example
	* // import qm module
	* var qm = require("qminer");
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "People",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Gendre", type: "string" },
	*            { name: "Age", type: "int" }
	*        ]
	*    }]
	* });
	* // create a feature space containing a categorical and numeric feature extractor
	* var ftr = new qm.FeatureSpace(base, [
	*    { type: "numeric", source: "People", field: "Age" },
	*    { type: "categorical", source: "People", field: "Gendre", values: ["Male", "Female"] }
	* ]);
	* // get the name of the feature extractor with index 1
	* var extractorName = ftr.getFeatureExtractor(1); // returns "Categorical[Gendre]"
	* base.close();
	*/
	//# exports.FeatureSpace.prototype.getFeatureExtractor = function (idx) { return ''; };
	JsDeclareFunction(getFeatureExtractor);

	/**
	* Gives the name of the feature at the given position.
	* @param {number} idx - The index of the feature in feature space (zero based).
	* @returns {String} The name of the feature at the position idx.
	* @example
	* // import qm module
	* var qm = require("qminer");
	* // create a base containing the store Class. Let the Name field be the primary field.
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Class",
	*        fields: [
	*            { name: "Name", type: "string", primary: true },
	*            { name: "StudyGroups", type: "string_v" }
	*        ]
	*    }]
	* });
	* // add some records to the store
	* base.store("Class").push({ Name: "Dean", StudyGroups: ["A", "D"] });
	* base.store("Class").push({ Name: "Chang", StudyGroups: ["B", "D"] });
	* base.store("Class").push({ Name: "Magnitude", StudyGroups: ["B", "C"] });
	* base.store("Class").push({ Name: "Leonard", StudyGroups: ["A", "B"] });
	* // create a feature space containing the multinomial feature extractor
	* var ftr = new qm.FeatureSpace(base, [
	* { type: "text", source: "Class", field: "Name" },
	* { type: "multinomial", source: "Class", field: "StudyGroups", values: ["A", "B", "C", "D"] }
	* ]);
	* // get the feature at position 2
	* var feature = ftr.getFeature(2); // returns "C", because the text extractor has no features at the moment
	* // update the feature space with the records of the store; see the method updateRecords
	* ftr.updateRecords(base.store("Class").allRecords);
	* // get the feature at position 2
	* var feature2 = ftr.getFeature(2); // returns "magnitude"
	* base.close();
	*/
	//# exports.FeatureSpace.prototype.getFeature = function (idx) { return ''; };
	JsDeclareFunction(getFeature);

    	/**
	* Performs the inverse operation of ftrVec. Works only for numeric feature extractors.
	* @param {(module:la.Vector | Array.<Object>)} ftr - The feature vector or an array with feature values.
	* @returns {module:la.Vector} The inverse of ftr as vector.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "TheWitcherSaga",
	*        fields: [
	*            { name: "Title", type: "string" },
	*            { name: "YearOfRelease", type: "int" },
	*            { name: "EnglishEdition", type: "bool" }
	*        ]
	*    }]
	* });
	* // put some records in the store
	* base.store("TheWitcherSaga").push({ Title: "Blood of Elves", YearOfRelease: 1994, EnglishEdition: true });
	* base.store("TheWitcherSaga").push({ Title: "Time of Contempt", YearOfRelease: 1995, EnglishEdition: true });
	* base.store("TheWitcherSaga").push({ Title: "Baptism of Fire", YearOfRelease: 1996, EnglishEdition: true });
	* base.store("TheWitcherSaga").push({ Title: "The Swallow's Tower", YearOfRelease: 1997, EnglishEdition: false });
	* base.store("TheWitcherSaga").push({ Title: "Lady of the Lake", YearOfRelease: 1999, EnglishEdition: false });
	* base.store("TheWitcherSaga").push({ Title: "Season of Storms", YearOfRelease: 2013, EnglishEdition: false });
	* // create a feature space with the numeric feature extractor and update the feature space with the records in store
	* // for update, look the method updateRecords in feature space
	* var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "TheWitcherSaga", field: "YearOfRelease", normalize: true });
	* ftr.updateRecords(base.store("TheWitcherSaga").allRecords);
	* // get a feature vector for the second record
	* // because of the numeric feature extractor having normalize: true and of the records update of feature space, the values
	* // are not equal to those of the records, i.e. the value 1995 is now 0.105263 
	* var ftrVec = ftr.extractVector(base.store("TheWitcherSaga")[1]);
	* // get the inverse of the feature vector
	* // the function returns the values to their first value, i.e. 0.105263 returns to 1995
	* var inverse = ftr.invertFeatureVector(ftrVec); // returns a vector [1995]
	* base.close();
	*/
	//# exports.FeatureSpace.prototype.invertFeatureVector = function (ftr) { return Object.create(require('qminer').la.Vector.prototype); };
	JsDeclareFunction(invertFeatureVector);

	/**
	* Calculates the inverse of a single feature using a specific feature extractor.
	* @param {number} idx - The index of the specific feature extractor.
	* @param {Object} val - The value to be inverted.
	* @returns {Object} The inverse of val using the feature extractor with index idx.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base containing one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "TheWitcherSaga",
	*        fields: [
	*            { name: "Title", type: "string" },
	*            { name: "YearOfRelease", type: "int" },
	*            { name: "EnglishEdition", type: "bool" }
	*        ]
	*    }]
	* });
	* // put some records in the store
	* base.store("TheWitcherSaga").push({ Title: "Blood of Elves", YearOfRelease: 1994, EnglishEdition: true });
	* base.store("TheWitcherSaga").push({ Title: "Time of Contempt", YearOfRelease: 1995, EnglishEdition: true });
	* base.store("TheWitcherSaga").push({ Title: "Baptism of Fire", YearOfRelease: 1996, EnglishEdition: true });
	* base.store("TheWitcherSaga").push({ Title: "The Swallow's Tower", YearOfRelease: 1997, EnglishEdition: false });
	* base.store("TheWitcherSaga").push({ Title: "Lady of the Lake", YearOfRelease: 1999, EnglishEdition: false });
	* base.store("TheWitcherSaga").push({ Title: "Season of Storms", YearOfRelease: 2013, EnglishEdition: false });
	* // create a feature space with the numeric feature extractor and update the feature space with the records in store
	* // for update, look the method updateRecords in feature space
	* var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "TheWitcherSaga", field: "YearOfRelease", normalize: true });
	* ftr.updateRecords(base.store("TheWitcherSaga").allRecords);
	* // because of the numeric feature extractor having normalize: true and of the records update of feature space, 
	* // the values are not equal to those of the records 
	* // invert the value 0 using the numeric feature extractor
	* var inverse = ftr.invertFeature(0, 0); // returns the value 1994
	* base.close();
	*/
	//# exports.FeatureSpace.prototype.invertFeature = function (idx, val) {};
	JsDeclareFunction(invertFeature);

	/**
	* Filters the vector to keep only the elements from the feature extractor.
	* @param {(module:la.Vector | module:la.SparseVector)} vec - The vector from where the function filters the elements.
	* @param {number} idx - The index of the feature extractor.
	* @param {boolean} [keepOffset = 'true'] - For keeping the original indexing in the new vector.
	* @returns {(module:la.Vector | module:la.SparseVector)} 
	* <br>1. module:la.Vector, if vec is of type module:la.Vector.
	* <br>2. module:la.SparseVector, if vec is of type module:la.SparseVector.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a new base with one store
	* var base = new qm.Base({
	*    mode: "createClean",
	*    schema: [{
	*        name: "Academics",
	*        fields: [
	*            { name: "Name", type: "string" },
	*            { name: "Age", type: "int" },
	*            { name: "Gendre", type: "string" },
	*            { name: "Skills", type: "string_v" }
	*        ]
	*    }]
	* });
	* // create a new feature space
	* var ftr = new qm.FeatureSpace(base, [
    *     { type: "numeric", source: "Academics", field: "Age" },
    *     { type: "categorical", source: "Academics", field: "Gendre", values: ["Male", "Female"] },
	*     { type: "multinomial", source: "Academics", field: "Skills", values: ["Mathematics", "Programming", "Philosophy", "Languages", "Politics", "Cooking"] }
	*     ]);
	* // create a new dense vector
	* var vec = new qm.la.Vector([40, 0, 1, 0, 1, 1, 1, 0, 0]);
	* // filter the elements from the second feature extractor
	* var vec2 = ftr.filter(vec, 1); // returns vector [0, 0, 1, 0, 0, 0, 0, 0, 0]
	* // filter the elements from the second feature extractor, without keeping the offset
	* var vec3 = ftr.filter(vec, 1, false); // returns vector [0, 1]
	* // create a new sparse vector
	* var spVec = new qm.la.SparseVector([[0, 40], [2, 1], [4, 1], [5, 1], [6, 1]]);
	* // filter the elements from the second feature extractor
	* var spVec2 = ftr.filter(spVec, 1); // returns sparse vector [[2, 1]]
	* // filter the elements from the second feature extractor, without keeping the offset
	* var spVec3 = ftr.filter(spVec, 1, false); // returns sparse vector [[1, 1]]
	* base.close();
	*/
	//# exports.FeatureSpace.prototype.filter = function (vec, idx, keepOffset) { return (vec instanceof require('qminer').la.Vector) ? require('qminer').la.Vector : require('qminer').la.SparseVector; };
    JsDeclareFunction(filter);

	/**
	* Extracts string features from the record.
	* @param {module:qm.Record} rec
	* @returns {Array.<string>} An array containing the strings gained by the extractor.
	* @ignore
	*/
	//# exports.FeatureSpace.prototype.extractStrings = function (rec) {return ['']; }; 
    JsDeclareFunction(extractStrings);

private:
    static TQm::PFtrExt NewFtrExtFromFunc(const TWPt<TQm::TBase>& Base, v8::Local<v8::Object>& Settings, v8::Isolate* Isolate) {
    	PJsonVal ParamVal = TNodeJsUtil::GetObjProps(Settings);
    	v8::Handle<v8::Function> Func = v8::Handle<v8::Function>::Cast(Settings->Get(v8::String::NewFromUtf8(Isolate, "fun")));
    	return TNodeJsFuncFtrExt::NewFtrExt(Base, ParamVal, Func, Isolate);
    }
};

#endif

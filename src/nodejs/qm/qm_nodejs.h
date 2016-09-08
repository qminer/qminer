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
* QMiner module.
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
    * @returns {module:qm.Base} The newly created base.
    */
    //# exports.create = function (configPath, schemaPath, clear) { return Object.create(require('qminer').Base.prototype); }
    JsDeclareFunction(create);

    /**
    * Opens a base.
    * @param {string} [configPath='qm.conf'] - The configuration file path.
    * @param {boolean} [readOnly=false] - Open in read-only mode.
    * @returns {module:qm.Base} The loaded base.
    */
    //# exports.open = function (configPath, readOnly) { return Object.create(require('qminer').Base.prototype); }
    JsDeclareFunction(open);
    
    /**
    * Set verbosity of QMiner internals.
    * @param {number} [level=0] - verbosity level. Possible options:
    * <br>1. `0` - No output, 
    * <br>2. `1` - Log output, 
    * <br>3. `2` - Log and debug output.
    */
    //# exports.verbosity = function (level) { }
    JsDeclareFunction(verbosity);

    /**
    * Returns an JSON with two properties: "byClass" and "total". The "byClass" value is a JSON where
    * each key is a class ID and each value is of the form { newFromCpp: number, newFromJs: number, destructorCalls: number}
    * and the value of "total" is of the same form (aggregated over "byClass")    
    */
    //# exports.stats = function () { }
    JsDeclareFunction(stats);
    
    /**
    * @typedef {Object} QMinerFlags
    * The object containing the QMiner compile flags.
    * @property {string} buildTime - The module build time.
    * @property {boolean} win - True, if the module is compiled for Windows.
    * @property {boolean} linux - True, if the module is compiled for Linux.
    * @property {boolean} darwin - True, if the module is compiled for Darwin.
    * @property {boolean} x86 - True, if the module is compiled for x86 system.
    * @property {boolean} x64 - True, if the module is compiled for x64 system.
    * @property {boolean} omp - True, if the module is compiled for omp.
    * @property {boolean} debug - True, if the module is compiled for debug mode.
    * @property {boolean} gcc - True, if the module is compiled for gcc.
    * @property {boolean} clang - True, if the module is compiled for clang.
    * @property {boolean} blas - True, if the module is compiled with Blas.
    * @property {boolean} blas_intel - True, if the module is compiled with Intel Blas.
    * @property {boolean} blas_amd - True, if the module is compiled with AMD Blas.
    * @property {boolean} blas_openblas - True, if the module is compiled with OpenBLAS.
    * @property {boolean} lapacke - True, if the module is compiled with Lapacke. 
    */

    /**
    * Returns an object with all compile flags. Type {@link module:qm~QMinerFlags}.
    * @example
    * // import qm module
    * var qm = require('qminer');
    * // get the compile flags
    * var flags = qm.flags;
    */
    //# exports.flags = { buildTime: "", win: true, linux: true, darwin: true, x86: true, x64: true, omp: true, debug: true, gcc: true, clang: true, blas: true, blas_intel: true, blas_amd: true, blas_openblas: true, lapacke: true };
    JsDeclareProperty(flags);

    /**
    * Returns the module version.
    * @returns {string} The module version.
    * @example
    * // import qm module
    * var qm = require('qminer');
    * // get the module version
    * var version = qm.version;
    */
    //# exports.version = '';
};

///////////////////////////////
// NodeJs QMiner Base

/**
* @typedef {Object} BaseConstructorParam
* Base constructor parameter used for {@link module:qm.Base}.
* @property  {string} [mode='openReadOnly'] - Base access mode. Can be one of the following:
* <br>1. `'create'` - Sets up the db folder, 
* <br>2. `'createClean'` - Cleans db folder and then sets it up, 
* <br>3. `'open'` - Opens the db with read/write permissions, 
* <br>4. `'openReadOnly'` - Opens the db in read only mode.
* @property  {number} [indexCache=1024] - The ammount of memory reserved for indexing (in MB).
* @property  {number} [storeCache=1024] - The ammount of memory reserved for store cache (in MB).
* @property  {string} [schemaPath=''] - The path to schema definition file.
* @property  {Array<module:qm~SchemaDef>} [schema=[]] - Schema definition object array.
* @property  {string} [dbPath='./db/'] - The path to db directory.
*/

/**
* @typedef {Object} SchemaDef
* Store schema definition used in {@link module:qm~BaseConstructorParam}.
* @property {string} name - The name of the store. Store name can be composed by from English letters, numbers, _ or $ characters. It can only begin with a character.
* @property {Array<module:qm~SchemaFieldDef>} fields - The array of field descriptors. 
* @property {Array<module:qm~SchemaJoinDef>} [joins=[]] - The array of join descriptors, used for linking records from different stores.
* @property {Array<module:qm~SchemaKeyDef>} [keys=[]] - The array of key descriptors. Keys define how records are indexed, which is needed for search using the query language.
* @property {module:qm~SchemaTimeWindowDef} [timeWindow] - Time window description. Stores can have a window, which is used by garbage collector to delete records once they fall out of the time window. Window can be defined by number of records or by time.
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
* @typedef {Object} SchemaFieldDef
* Store schema field definition used in {@link module:qm~SchemaDef}.
* @property {string} name - The name of the field.
* @property {string} type - The type of the field. Possible options:
* <br>1. `'int'` - Signed 32-bit integer,
* <br>2. `'uint64'` - Unsigned 64-bit integer,
* <br>3. `'int_v'` - Array of signed 32-bit integers,
* <br>4. `'string'` - String,
* <br>5. `'string_v'` - Array of strings,
* <br>6. `'bool'` - Boolean,
* <br>7. `'float'` - Double precision flating point number,
* <br>8. `'float_pair'` - A pair of floats, useful for storing geo coordinates,
* <br>9. `'float_v'` - Array of floats,
* <br>10. `'datetime'` - Date and time format, stored in a form of miliseconds since 1600,
* <br>11. `'num_sp_v'` - Array of [`int`, `float`] pairs. See constructor array for {@link module:la.SparseVector}.
* @property {boolean} [primary=false] - Field which can be used to identify record. There can be only one primary field in a store. There can be at most one record for each value of the primary field. Currently following fields can be marked as primary: `int`, `uint64`, `string`, `float`, `datetime`. Primary fields of type `string` are also used for record querying using {@link module:qm.Store#recordByName}.
* @property {boolean} [null=false] - When set to true, null is a possible value for a field (allow missing values).
* @property {string} [store='memory'] - Defines where to store the field. Possible options 
* <br>1. `'memory'` - Stores the values in RAM. 
* <br>2 `'cache'` - Stores the values on disk, with a layer of FIFO cache in RAM, storing the most recently used values.
* @property {Object} [default] - Default value for field when not given for a new record.
* @property {boolean} [codebook=false] - Useful when many records have only few different values of this field. If set to true, then a separate table of all values is kept, and records only point to this table (replacing variable string field in record serialisation with fixed-length integer). Useful to decrease memory footprint, and faster to update. (STRING FIELD TYPE SPECIFIC).
* @property {boolean} [shortstring=false] - Useful for string shorter then 127 characters (STRING FIELD TYPE SPECIFIC).
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
* @typedef {Object} SchemaJoinDef
* Store schema join definition used in {@link module:qm~SchemaDef}.
* @property {string} name - The name of the join.
* @property {string} type - Join types. Possible options:
* <br>1. `'field'` - Points to zero or one record and is implemented as an additional hidden field of type `uint64`, which can hold the ID of the record it links to. Accessing the records join returns a record.
* <br>2. `'index'` - Point to any number of records and is implemented using the inverted index, where for each record a list (vector) of linked records is kept. Accessing the records join returns a record set. 
* <b>Important:</b> The records given to this join field must be in an array.
*
* @property {string} store - The store name from which the linked records are.
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
*
* // Adds a movie, sets the 'director' join, updates the index join of 'Jim Jarmusch'
* base.store('Movies').push({ title: 'Coffee and Cigarettes', director: { name: 'Jim Jarmusch' } });
* // Adds movie, automatically adds 'Lars von Trier' to People, sets the 'director' join
* // and 'directed' inverse join (automatically)
* base.store('Movies').push({ title: 'Dogville', director: { name: 'Lars von Trier' } });
*
* // Adds a person, sets the 'directed' join with multiple movies ('directed' is of type 'index', movies must be given in an array)
* base.store('People').push({ name: 'Christopher Nolan', directed: [{ title: 'Inception' }, { title: 'Interstellar' }] });
*
* var movie = base.store('Movies')[0]; // get the first movie (Broken Flowers)
* // Each movie has a property corresponding to the join name: 'director'. 
* // Accessing the property returns a {@link module:qm.Record} from the store People.
* var person = movie.director; // get the director
* var personName = person.name; // get person's name ('Jim Jarmusch')
*
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
* @typedef {Object} SchemaKeyDef
* Store schema key definition used in {@link module:qm~SchemaDef}.
* @property {string} field - The name of the field that will be indexed.
* @property {string} type - Key type. Possible options:
* <br>1. `'value'` - Indexes records using an inverted index using full value of the field (no processing).
*  The key type supports `'string'`, `'string_v'` and `'datetime'` fields types.
* <br>2. `'text'` - Indexes string fields by using a tokenizer and text processing. Supported by `'string'` fields.
* <br>3. `'location'`- Indexes records as points on a sphere and enables nearest-neighbour queries. Supported by `'float_pair'` type fields.
* @property {string} [name] - Allows using a different name for the key in search queries. This allows for multiple keys to be put against the same field. Default value is the name of the field.
* @property {string} [vocabulary] - Defines the name of the vocabulary used to store the tokens or values. This can be used indicate to several keys to use the same vocabulary, to save on memory. Supported by `'value'` and `'text'` keys.
* @property {string} [tokenize] - Defines the tokenizer that is used for tokenizing the values stored in indexed fields. Tokenizer uses same parameters as in bag-of-words feature extractor. Default is english stopword list and no stemmer. Supported by `'text'` keys.
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
* base.search({$from : 'People', name: 'Smith'}); // Returns the empty record set
* // search based on text indexing
* base.search({$from : 'People', nameText: 'Smith'}); // Returns both records
* base.close();
*/

/**
* @typedef {Object} SchemaTimeWindowDef
* Stores can have a window, which is used by garbage collector to delete records once they
* fall out of the time window. Window can be defined by number of records or by time.
* Window defined by parameter window, its value being the number of records to be kept. Used in {@link module:qm~SchemaDef}.
* <br><b>Important:</b> {@link module:qm.Base#garbageCollect} must be called manually to remove records outside time window.
* @property {number} duration - The size of the time window (in number of units).
* @property {string} unit - Defines in which units the window size is specified. Possible options are `'second'`, `'minute'`, `'hour'`, `'day'`, `'week'` or `'month'`.
* @property {string} [field] - Name of the datetime field, which defines the time of the record. In case it is not given, the insert time is used in its place.
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
* base.store("TestStore").allRecords.length; // 5
* // clean base with garbage collector
* base.garbageCollect();
* // check number of records in store
* base.store("TestStore").allRecords.length; // 3
* base.close();
*
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
* base.store("TestStore").allRecords.length; // 5
* // clean base with garbage collector
* base.garbageCollect();
* // check number of records in store
* base.store("TestStore").allRecords.length; // 2
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
* @classdesc Represents the database and holds stores.
* @class
* @param {module:qm~BaseConstructorParam} paramObj - The base constructor parameter object.
* @example
* // import qm module
* var qm = require('qminer');
* // using a constructor, in open mode
* var base = new qm.Base({ mode: 'open' });
* base.close();
*/
//# exports.Base = function (paramObj) { return Object.create(require('qminer').Base.prototype); };
class TNodeJsBase : public node::ObjectWrap {
    friend class TNodeJsUtil;
private:
    static v8::Persistent<v8::Function> Constructor;
    ~TNodeJsBase() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; }
public:
    static const int MAX_BASES;
    static void Init(v8::Handle<v8::Object> Exports);
    static const TStr GetClassId() { return "Base"; }
    // wrapped C++ object
    TWPt<TQm::TBase> Base;
    // C++ constructor
    TNodeJsBase(const TWPt<TQm::TBase>& Base_) : Base(Base_) { Watcher = TNodeJsBaseWatcher::New(); }
    TNodeJsBase(const TStr& DbPath, const TStr& SchemaFNm, const PJsonVal& Schema,
        const bool& Create, const bool& ForceCreate, const bool& ReadOnly,
        const bool& UseStrictFldNames, const uint64& IndexCache, const uint64& StoreCache);
    // Object that knows if Base is valid
    PNodeJsBaseWatcher Watcher;
private:        
    // parses arguments, called by javascript constructor 
    static TNodeJsBase* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);
private:
    /**
    * Closes the database.
    * @returns {null} No value is returned.
    * @example
    * // import qm module
    * var qm = require('qminer');
    * // using a constructor, in open mode
    * var base = new qm.Base({ mode: 'open' });
    * // close the database
    * base.close();
    */
    //# exports.Base.prototype.close = function () { return null; }
    JsDeclareFunction(close);

    /**
    * Checks if the base is closed.
    * @returns {Boolean} Returns `true`, if the base is closed.
    * @example
    * // import qm module
    * var qm = require('qminer');
    * // using a constructor, in open mode
    * var base = new qm.Base({ mode: 'open' });
    * // check if the base is closed
    * var closed = base.isClosed();
    * // close the database
    * base.close();
    */
    //# exports.Base.prototype.isClosed = function () { return true; }
    JsDeclareFunction(isClosed);

    /**
     * Returns the store with the specified name.
     * @param {string} name - Store name.
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
     * var store = base.store("KwikEMart"); // returns the store with the name "KwikEMart"
     * base.close();
     */
    //# exports.Base.prototype.store = function (name) { return Object.create(require('qminer').Store.prototype); }
    JsDeclareFunction(store);

    /**
     * Checks if there is a store.
     * @param {string} name - Store name.
     * @returns {boolean} True, if there exists a store with the store `name`. Otherwise, false.
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
     * var exists = base.isStore("KwikEMart");    // true
     * base.close();
     */
    //# exports.Base.prototype.isStore = function (name) { return false; }
    JsDeclareFunction(isStore);

    /**
     * Returns a list of store descriptors.
     * @returns {Array.<object>} An array of store descriptors. The store descriptor `storeDesc` contains the properties:
     * <br>1. `storeDesc.storeId` - The store ID. Type `number`.
     * <br>2. `storeDesc.storeName` - Store name. Type `string`.
     * <br>3. `storeDesc.storeRecords` - Number of records in store. Type `number`.
     * <br>4. `storeDesc.fields` - The store field schema. Type <code>Array of <a href="module-qm.html#~SchemaFieldDef">module:qm.SchemaFieldDef</a></code>.
     * <br>5. `storeDesc.keys` - The store key schema. Type <code>Array of <a href="module-qm.html#~SchemaKeyDef">module:qm.SchemaKeyDef</a></code>.
     * <br>6. `storeDesc.joins` - The store join schema. Type <code>Array of <a href="module-qm.html#~SchemaJoinDef">module:qm.SchemaJoinDef</a></code>.
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
     * // get the list of store descriptors
     * var exists = base.getStoreList();
     * base.close();
     */
    //# exports.Base.prototype.getStoreList = function () { return [{storeId: 0, storeName:'', storeRecords: 0, fields: [{}], keys: [{}], joins: [{}]}]; }
    JsDeclareFunction(getStoreList);

    /**
    * Creates a new store.
    * @param {Array.<module:qm~SchemaDef>} storeDef - The definition of the store(s).
    * @param {number} [storeSizeInMB = 1024] - The reserved size of the store(s).
    * @returns {(module:qm.Store | Array.<module:qm.Store>)} - Returns a store or an array of stores (if the schema definition was an array).
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
    * @typedef {object} QueryObject
    * The object used for querying records with {@link module:qm.Base#search}.
    * How to construct a query is found on the <a href="https://github.com/qminer/qminer/wiki/Query-Language">QMiner Wiki page</a>.
    */

    /**
    * Makes a query search and returns a record set.
    * @param {module:qm~QueryObject} query - Query language JSON object.
    * @returns {module:qm.RecordSet} The record set that matches the search criterion.
    */
    //# exports.Base.prototype.search = function (query) { return Object.create(require('qminer').RecordSet.prototype); }

    JsDeclareFunction(search);   

    /**
    * Calls qminer garbage collector to remove records outside time windows. For application example see {@link module:qm~SchemaTimeWindowDef}.
    */
    //# exports.Base.prototype.garbageCollect = function () { }
    JsDeclareFunction(garbageCollect);

    /**
    * Base saves dirty data given some time window.
    * @param {number} [window=500] - Length of available time window in miliseconds.
    * @returns {number} Number of records it flushed.
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
    * // call the garbage collector
    * base.partialFlush();
    * base.close();
    */
    //# exports.Base.prototype.partialFlush = function () { return 0; }

    JsDeclareFunction(partialFlush);

    /**
    * @typedef {object} PerformanceStat
    * The performance statistics used to describe {@link module:qm~PerformanceStatBase} and {@link module:qm~PerformanceStatStore}.
    * @property {number} alloc_count - \\ TODO: Add the description
    * @property {number} alloc_size - \\ TODO: Add the description
    * @property {number} alloc_unused_size - \\ TODO: Add the description
    * @property {number} avg_get_len - \\ TODO: Add the description
    * @property {number} avg_put_len - \\ TODO: Add the description
    * @property {number} avg_get_new_len - \\ TODO: Add the description
    * @property {number} dels - \\ TODO: Add the description
    * @property {number} gets - \\ TODO: Add the description
    * @property {number} puts - \\ TODO: Add the description
    * @property {number} puts_new - \\ TODO: Add the description
    * @property {number} released_count - \\ TODO: Add the description
    * @property {number} released_size - \\ TODO: Add the description
    * @property {number} size_changes - \\ TODO: Add the description
    */

    /**
    * @typedef {object} PerformanceStatStore
    * The performance statistics of the store found in {@link module:qm~PerformanceStatBase}.
    * @property {string} name - Store name.
    * @property {module:qm~PerformanceStat} blob_storage_memory - \\ TODO: Add the description
    * @property {module:qm~PerformanceStat} blob_storage_cache - \\ TODO: Add the description
    */

    /**
    * @typedef {object} PerformanceStatBase
    * The performance statistics that is returned by {@link module:qm.Base#getStats}.
    * @property {Array.<module:qm~PerformanceStatStore>} stores - The performance statistics of the stores in base. \\ TODO: Check if this is right
    * @property {object} gix_stats - The statistics of the base. \\ TODO: Check if this is right
    * @property {number} gix_stats.avg_len - The average length. \\ TODO: Check if this is right
    * @property {number} gix_stats.cache_all - The number of cache. \\ TODO: Check if this is right
    * @property {number} gix_stats.cache_all_loaded_perc - \\ TODO: Add the description
    * @property {number} gix_stats.cache_dirty - \\ TODO: Add the description
    * @property {number} gix_stats.cache_dirty_loaded_perc - \\ TODO: Add the description
    * @property {number} gix_stats.mem_sed - \\ TODO: Add the description
    * @property {module:qm~PerformanceStat} gix_blob - \\ TODO: Add the description
    */

    /**
    * Retrieves performance statistics for qminer.
    * @returns {module:qm~PerformanceStatBase} The performance statistics. 
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
    * // call the garbage collector
    * base.getStats();
    * base.close();
    */
    //# exports.Base.prototype.getStats = function () { }
    JsDeclareFunction(getStats);

    /**
    * Gets the stream aggregate of the given name.
    * @param {string} saName - The name of the stream aggregate.
    * @returns {module:qm.StreamAggr} The stream aggregate whose name is `saName`.
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
    * // get the stream aggregate with the name 'Laser'
    * var streamAggr = base.getStreamAggr('WaveLengthLaser');
    * base.close();
    */
    //# exports.Base.prototype.getStreamAggr = function (saName) { return Object.create(require('qminer').StreamAggr.prototype); }
    JsDeclareFunction(getStreamAggr);

    /**
    * Gets an array of the stream aggregate names in the base.
    * @returns {Array.<string>} The array containing the stream aggregate names.
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
    * // get the stream aggregates names
    * var streamAggrNames = base.getStreamAggrNames();
    * base.close();
    */
    //# exports.Base.prototype.getStreamAggrNames = function () { return [""]; }
    JsDeclareFunction(getStreamAggrNames);  
    //!JSIMPLEMENT:src/qminer/qminer.js    
};


///////////////////////////////
// NodeJs-Qminer-Store

/**
* Stores are containers of records. <br>
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
    ~TNodeJsStore() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; }
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
    * @returns {(module:qm.Record | null)} Returns the record. If the record doesn't exist, it returns `null`.
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
    * <br>1. `rec` - The current record. Type {@link module:qm.Record}.
    * <br>2. `idx` - The index of the current record (<i>optional</i>). Type `number`.
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
    * base.store("Class").each(function (rec) { rec.StudyGroup = "A"; });   // all records in Class are now in study group A
    * base.close();
    */
    //# exports.Store.prototype.each = function (callback) { return Object.create(require('qminer').Store.prototype); }
    JsDeclareFunction(each);

    /**
    * Creates an array of function outputs created from the store records.
    * @param {function} callback - Function that generates the array. It takes two parameters:
    * <br>1. `rec` - The current record. Type {@link module:qm.Record}.
    * <br>2. `idx` - The index of the current record (<i>optional</i>). Type `number`.
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
    * @param {object} rec - The added record. The record must be a object corresponding to store schema created at store creation using {@link module:qm~SchemaDef}.
    * @param {boolean} [triggerEvents=true] - If true, all stream aggregate callbacks `onAdd` will be called after the record is inserted. If false, no stream aggregate will be updated.
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
    * @param {object} obj - An object describing the record.
    * @returns {module:qm.Record} The record created by `obj` and the store.
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
    //# exports.Store.prototype.newRecord = function (obj) { return Object.create(require('qminer').Record.prototype); };
    JsDeclareFunction(newRecord);

    /**
    * Creates a new record set out of the records in store.
    * @param {module:la.IntVector} idVec - The integer vector containing the IDs of selected records.
    * @returns {module:qm.RecordSet} The record set that contains the records gained with `idVec`.
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
    * @param {number} sampleSize - The size of sample.
    * @returns {module:qm.RecordSet} Returns a record set containing `sampleSize` random records.
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
    * @param {string} fieldName - The field name.
    * @returns {object} The object containing the details of the field. The properties are: 
    * <br>1. `id` - The ID of the field. Type ´number´.
    * <br>2. `name` - The name of the field. Type `string`.
    * <br>3. `type` - The type of the field. Type `string`.
    * <br>4. `nullable` - If the field value can be null. Type `boolean`.
    * <br>5. `internal` - If the field is internal. Type `boolean`.
    * <br>6. `primary` - If the field is primary. Type `boolean`.
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
    * // { id: 0, name: "Name", type: "string", primary: true, nullable: false, internal: false }
    * var details = base.store("People").field("Name");
    * base.close();
    */
    //# exports.Store.prototype.field = function (fieldName) { return { id: 0, name:'', type:'', primary: true, internal: true, nullable: true }; }; 
    JsDeclareFunction(field);

    /**
    * Checks if the field is of numeric type.
    * @param {string} fieldName - The field name.
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
    * @param {string} fieldName - The field name.
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
    * @param {string} fieldName - The field name.
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
    * @typedef {object} DetailKeyObject
    * The details about the key object used in {@link module:qm.Store#key} and {@link module:qm.Store#keys}.
    * @property {module:la.IntVector} fq - The frequency.
    * @property {module:la.StrVector} vocabulary - The vocabulary.
    * @property {string} name - The key name.
    * @property {module:qm.Store} store - The store.
    */

    /**
    * Returns the details of the selected key as a object.
    * @param {string} keyName - The key name.
    * @returns {module:qm~DetailKeyObject} The object containing the details of the key.
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
    //# exports.Store.prototype.key = function (keyName) { return { fq: Object.create(require('qminer').la.IntVector.prototype), vocabulary: Object.create(require('qminer').la.StrVector.prototype), name:'', store: Object.create(require('qminer').Store.prototype) }; }
    JsDeclareFunction(key);

    /**
    * Resets all stream aggregates.
    * @example
    * // import qm module
    * var qm = require('qminer');
    * // create a simple base containing one store
    * var base = new qm.Base({
    *    mode: "createClean",
    *    schema: [{
    *        name: "Laser",
    *        fields: [
    *            { name: "Time", type: "datetime" },
    *            { name: "WaveLength", type: "float" }
    *        ]
    *    }]
    * });
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
    * // reset the stream aggregates on store "Laser"
    * base.store("Laser").resetStreamAggregates();
    * base.close();
    */
    //# exports.Store.prototype.resetStreamAggregates = function () { }
    JsDeclareFunction(resetStreamAggregates);

    /**
    * Returns an array of the stream aggregates names connected to the store.        
    * @returns {Array.<string>} An array of stream aggregates names.
    * @example
    * // import qm module
    * var qm = require('qminer');
    * // create a simple base containing one store
    * var base = new qm.Base({
    *    mode: "createClean",
    *    schema: [{
    *        name: "Laser",
    *        fields: [
    *            { name: "Time", type: "datetime" },
    *            { name: "WaveLength", type: "float" }
    *        ]
    *    }]
    * });
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
    * // get the stream aggregates on store "Laser"
    * base.store("Laser").getStreamAggrNames();
    * base.close();
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
    //# exports.Store.prototype.toJSON = function () { return { storeId:0, storeName:'', storeRecords:0, fields:[{}], keys:[{}], joins:[{}] }; };
    JsDeclareFunction(toJSON);

    /**
    * Deletes the first records in the store.
    * @param {number} [num] - The number of deleted records. If the number is given, the first `num` records will be deleted.
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
    * @param {string} fieldName - The field name. Field must be of one-dimensional type, e.g. `int`, `float`, `string`...
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
    * var companyNames = base.store("Companies").getVector("Name"); // returns a vector ["DC Comics", "DC Shoes", "21st Century Fox"]
    * base.close();
    */
    //# exports.Store.prototype.getVector = function (fieldName) { return Object.create(require('qminer').la.Vector.prototype); };
    JsDeclareFunction(getVector);

    /**
    * Gives a matrix containing the field values of each record.
    * @param {string} fieldName - The field name. Field mustn't be of type `string`.
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
    * @param {string} fieldName - The field name.
    * @returns {number | string | Array.<number> | Array.<string>} The `fieldName` value of the record with ID `recId`.
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
    * Calls `onAdd` callback on all stream aggregates.
    * @param {(module:qm.Record | number)} [arg] - The record or record ID which will be passed to `onAdd` callbacks. If the record or record ID is not provided, the last record will be used. Throws exception if the record cannot be provided.
    * <br>Defaults to the last {@link module:qm.Record} in store.
    */
    //# exports.Store.prototype.triggerOnAddCallbacks = function (arg) {};
    JsDeclareFunction(triggerOnAddCallbacks);

    /**
    * Gives the name of the store. Type `string`.
    */
    //# exports.Store.prototype.name = "";
    JsDeclareProperty(name);

    /**
    * Checks if the store is empty. Type `boolean`.
    */
    //# exports.Store.prototype.empty = true;
    JsDeclareProperty(empty);

    /**
    * Gives the number of records. Type `number`.
    */
    //# exports.Store.prototype.length = 0;
    JsDeclareProperty(length);

    /**
    * Creates a record set containing all the records from the store. Type {@link module:qm.RecordSet}.
    */
    //# exports.Store.prototype.allRecords = Object.create(require('qminer').RecordSet.prototype);
    JsDeclareFunction(allRecords);

    /**
    * Gives an array of all field descriptor objects. Type `Array of objects`, where the objects contain the properties:
    * <br>1. `id` - The ID of the field. Type ´number´.
    * <br>2. `name` - The name of the field. Type `string`.
    * <br>3. `type` - The type of the field. Type `string`.
    * <br>4. `nullable` - If the field value can be null. Type `boolean`.
    * <br>5. `internal` - If the field is internal. Type `boolean`.
    * <br>6. `primary` - If the field is primary. Type `boolean`.
    */
    //# exports.Store.prototype.fields = [{}];
    JsDeclareProperty(fields);

    /**
    * Gives an array of all join descriptor objects. Type `Array of objects`, where the objects contain the properties:
    * <br>1. `id` - The ID of the join. Type ´number´.
    * <br>2. `name` - The name of the join. Type `string`.
    * <br>2. `store` - The store the join was created in. Type `string`.
    * <br>2. `inverse` - The inverse join. Type `string`.
    * <br>3. `type` - The type of the field. Type `string`.
    * <br>4. `key` - The index key. Type {@link module:qm~DetailKeyObject}.
    */
    //# exports.Store.prototype.joins = [{}];
    JsDeclareProperty(joins);

    /**
    * Gives an array of all key descriptor objects. Type <code>Array of <a href="module-qm.html#~DetailKeyObject">module:qm~DetailKeyObject</a></code>.
    */
    //# exports.Store.prototype.keys = [{}];
    JsDeclareProperty(keys);

    /**
    * Returns the first record of the store. Type {@link module:qm.Record}.
    */
    //# exports.Store.prototype.first = Object.create(require('qminer').Record.prototype);
    JsDeclareFunction(first);

    /**
    * Returns the last record of the store. Type {@link module:qm.Record}.
    */
    //# exports.Store.prototype.last = Object.create(require('qminer').Record.prototype);
    JsDeclareFunction(last);

    /**
    * Returns an iterator for iterating over the store from start to end. Type {@link module:qm.Iterator}.
    */
    //# exports.Store.prototype.forwardIter = Object.create(require('qminer').Iterator.prototype);
    JsDeclareFunction(forwardIter);

    /**
    * Returns an iterator for iterating over the store form end to start. Type {@link module:qm.Iterator}.
    */
    //# exports.Store.prototype.backwardIter = Object.create(require('qminer').Iterator.prototype);
    JsDeclareFunction(backwardIter);

    /**
    * Gets the record with the given ID.
    * @param {number} recId - The id of the record.
    * @returns {module:qm.Record} The record with the ID equal to `recId`.
    * @ignore
    */
    //# exports.Store.prototype.store = function (recId) { };
    JsDeclIndexedProperty(indexId); 

    /**
    * Returns the base, in which the store is contained. Type {@link module:qm.Base}.
    */
    //# exports.Store.prototype.base = Object.create(require('qminer').Base.prototype);
    JsDeclareFunction(base);
    //!JSIMPLEMENT:src/qminer/store.js
};

///////////////////////////////
// NodeJs QMiner Record

/**
* Records are used for storing data in {@link module:qm.Store}. <br>
* <b>Factory pattern</b>: this class cannot be construced using the new keyword. This class is constructed
* when calling a specific method or attribute, e.g. using {@link module:qm.Store#push} to create a new record in 
* the store or {@link module:qm.Store#newRecord} to create a new record, that is not saved in the store.
* @class
*/
//# exports.Record = function () { return Object.create(require('qminer').qm.Record.prototype); }; 
class TNodeJsRec: public node::ObjectWrap {
    friend class TNodeJsUtil;
private:
    // Modified node framework: one record template per each base,storeId combination
    static TVec<TVec<v8::Persistent<v8::Function> > > BaseStoreIdConstructor;
    ~TNodeJsRec() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; }
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
    * Adds a join record `joinRecord` to join `joinName` (string) with join frequency `joinFrequency`.
    * @param {string} joinName - Join name.
    * @param {(module:qm.Record | number)} joinRecord - Joined record or its ID.
    * @param {number} [joinFrequency=1] - Frequency attached to the join.
    * @returns {module:qm.Record} The joined record.
    * @example
    * // import qm module
    * var qm = require('qminer');
    * // Create two stores
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
    * // Add a record to people and a record to movies
    * base.store('Movies').push({ title: 'Coffee and Cigarettes' });
    * base.store('People').push({ name: 'Jim Jarmusch' });
    *
    * // add a join between the added records
    * base.store('People')[0].$addJoin('directed', base.store('Movies')[0]);
    * base.close();
    */
    //# exports.Record.prototype.$addJoin = function (joinName, joinRecord, joinFrequency) { return Object.create(require('qminer').Record.prototype); }
    JsDeclareFunction(addJoin);

    /**
    * Deletes join record `joinRecord` from join `joinName` (string) with join frequency `joinFrequency`.
    * @param {string} joinName - Join name.
    * @param {(module:qm.Record | number)} joinRecord - Joined record or its ID.
    * @param {number} [joinFrequency=1] - Frequency attached to the join.
    * @returns {module:qm.Record} The joined record.
    * @example
    * // import qm module
    * var qm = require('qminer');
    * // Create two stores
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
    * // Add a record to people and a record to movies
    * base.store('Movies').push({ title: 'Coffee and Cigarettes', director: { name: 'Jim Jarmusch' } });
    * // delete the join between the added records
    * base.store('People')[0].$delJoin('directed', base.store('Movies')[0]);
    * base.close();
    */
    //# exports.Record.prototype.$delJoin = function (joinName, joinRecord, joinFrequency) { return Object.create(require('qminer').Record.prototype); }
    JsDeclareFunction(delJoin);

    /**
    * Creates a JSON version of the record.
    * @param {Boolean} [joinRecords=false] - Include joined records (only IDs).
    * @param {Boolean} [joinRecordFields=false] - Expand joined record fields.
    * @param {Boolean} [sysFields=true] - If set to true system fields, like $id, will be included.
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
    //# exports.Record.prototype.toJSON = function () { return {}; };
    JsDeclareFunction(toJSON);

    /**
    * Returns the id of the record. Type `number`.
    */
    //# exports.Record.prototype.$id = 0;
    JsDeclareProperty(id);

    /**
    * Returns the name of the record. Type `string`.
    */
    //# exports.Record.prototype.$name = "";
    JsDeclareProperty(name);

    /**
    * Returns the frequency of the record. Type `number`.
    */
    //# exports.Record.prototype.$fq = 0;
    JsDeclareProperty(fq);

    /**
    * Returns the store the record belongs to. Type {@link module:qm.Store}.
    */
    //# exports.Record.prototype.store = Object.create('qminer').Store.prototype;
    JsDeclareFunction(store);

    JsDeclareSetProperty(getField, setField);
    JsDeclareFunction(join);
    JsDeclareFunction(sjoin);
};

///////////////////////////////
// NodeJs QMiner Record Vector

/**
 * Vector of records by value.
 * @class
 * @param {module:fs.FIn} [arg] - Load vector from input stream.
 * @classdesc Vector storing records defined by value. Vector can be serialized and
 * iterated over. For storing records by reference use {@link module:qm.RecordSet} or 
 * {@link module:la.IntVector}.
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
 * // Create record vector
 * var recordVector = new qm.RecordVector(base);
 * // Add some records to the vector
 * recordVector.push(base.store("Philosophers").newRecord({ Name: "Plato", Era: "Ancient philosophy" }));
 * recordVector.push(base.store("Philosophers").newRecord({ Name: "Immanuel Kant", Era: "18th-century philosophy" }));
 * recordVector.push(base.store("Philosophers").newRecord({ Name: "Emmanuel Levinas", Era: "20th-century philosophy" }));
 * recordVector.push(base.store("Philosophers").newRecord({ Name: "Rene Descartes", Era: "17th-century philosophy" }));
 * recordVector.push(base.store("Philosophers").newRecord({ Name: "Confucius", Era: "Ancient philosophy" }));
 * // Iterate over all records
 * for (var i = 0; i < recordVector.length; i++) {
 *    var rec = recordVector[i];
 *    var tite = rec.Name + " (" + rec.Era + ")";
 * }
 * base.close();
 */
//# exports.RecordVector = function(arg) { return Object.create(require('qminer').qm.RecordVector.prototype) };
class TNodeJsRecByValV: public node::ObjectWrap {
    friend class TNodeJsUtil;
private:
    // Node framework
    static v8::Persistent<v8::Function> Constructor;
    ~TNodeJsRecByValV() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; }
public:
    // Node framework
    static void Init(v8::Handle<v8::Object> Exports);
    static const TStr GetClassId() { return "RecordVector"; }
    // Object that knows if Base is valid
    PNodeJsBaseWatcher Watcher;
    // C++ wrapped object
    TVec<TQm::TRec> RecV;
    // C++ constructors
    TNodeJsRecByValV(PNodeJsBaseWatcher _Watcher): Watcher(_Watcher) { }
    // JavaScript Constructor
    static TNodeJsRecByValV* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

private:
    /**
     * Adds a new record to the vector.
     * @param {module:qm.Record} rec - The added record. The record must be provided by value.
     * @returns {number} The position of the added record in the vector.
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
     * // Create record vector
     * var recordVector = new qm.RecordVector(base);
     * // Add some records to the vector
     * recordVector.push(base.store("Philosophers").newRecord({ Name: "Plato", Era: "Ancient philosophy" }));
     * base.close();
     */
    //# exports.RecordVector.prototype.push = function (rec) {};
    JsDeclareFunction(push);

    /**
     * Gives the number of records. Type `number`.
     */
    //# exports.RecordVector.prototype.length = 0;
    JsDeclareProperty(length);

    /**
    * Gets the record with the given ID.
    * @param {number} recN - The index of the record
    * @returns {module:qm.Record} The record at `recN` position.
    * @ignore
    */
    JsDeclIndexedProperty(indexId);

    /**
     * Saves the vector into the output stream.
     * @param {module:fs.FOut} fout - Output stream.
     * @returns {module:fs.FOut} The output stream `fout`.
     * @example
     * // import qm module
     * var qm = require('qminer');
     * var fs = require('qminer').fs;
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
     * // Create record vector
     * var recordVector = new qm.RecordVector(base);
     * // Add some records to the vector
     * recordVector.push(base.store("Philosophers").newRecord({ Name: "Plato", Era: "Ancient philosophy" }));
     * // save to disk
     * var fout = fs.openWrite('record_vector.bin');
     * recordVector.save(fout).close();
     * // load into a new vector
     * var fin = fs.openRead('record_vector.bin');
     * var recordVector2 = new qm.RecordVector(base, fin);
     * base.close();
     */
    //# exports.RecordVector.prototype.save = function(fout) { return Object.create(require('qminer').RecordVector.prototype); };
    JsDeclareFunction(save);
};

///////////////////////////////
// NodeJs QMiner Record Set

/**
* Record Set is a set of records. <br>
* <b>Factory pattern</b>: this class cannot be construced using the new keyword. This class is constructed
* when calling a specific method or attribute, e.g. using {@link module:qm.Store#allRecords} to get all the records
* in the store as a record set.
* @class
*/
//# exports.RecordSet = function () { return Object.create(require('qminer').qm.RecordSet.prototype); }
class TNodeJsRecSet: public node::ObjectWrap {
    friend class TNodeJsUtil;
private:
    // Node framework
    static v8::Persistent<v8::Function> Constructor;
    ~TNodeJsRecSet() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; }
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
    * Sorts the records according to record ID.
    * @param {number} [asc=-1] - If `asc` > 0, it sorts in ascending order. Otherwise, it sorts in descending order.  
    * @returns {module:qm.RecordSet} Self. Records are sorted according to record ID and `asc`.
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
    * @param {number} [asc=1] - If `asc` > 0, it sorts in ascending order. Otherwise, it sorts in descending order.
    * @returns {module:qm.RecordSet} Self. Records are sorted according to record weight and `asc`.
    */
    //# exports.RecordSet.prototype.sortByFq = function (asc) { return Object.create(require('qminer').RecordSet.prototype); }; 
    JsDeclareFunction(sortByFq);

    /**
    * Sorts the records according to a specific record field.
    * @param {string} fieldName - The field by which the sort will work.
    * @param {number} [arc=-1] - if `asc` > 0, it sorts in ascending order. Otherwise, it sorts in descending order.
    * @returns {module:qm.RecordSet} Self. Records are sorted according to `fieldName` and `arc`.
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
    * <br>1. `rec` - The first record.
    * <br>2. `rec2` - The second record.
    * <br>The function return type `boolean`.
    * @returns {module:qm.RecordSet} Self. The records are sorted according to the `callback` function.
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
    * <br>1. Contains only the records of the original with IDs between `minId` and `maxId`, if parameters are given.
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
    * <br>1. Contains only the records of the original with weights between `minFq` and `maxFq`, if parameters are given.
    * <br>2. Contains all the records of the original, if no parameter is given.
    */
    //# exports.RecordSet.prototype.filterByFq = function (minFq, maxFq) { return Object.create(require('qminer').RecordSet.prototype); };
    JsDeclareFunction(filterByFq);

    /**
    * Keeps only the records with a specific value of some field.
    * @param {string} fieldName - The field by which the records will be filtered.
    * @param {(string | number)} minVal -
    * <br>1. If the field type is a `string`, the exact string to compare. Type `number`.
    * <br>2. If the field type is a `number`, the minimal value for comparison. Type `number`.
    * <br>3. TODO Time field
    * @param {number} maxVal - Only in combination with `minVal` for non-string fields. The maximal value for comparison.
    * @returns {module:qm.RecordSet} Self. 
    * <br>1. If the `fieldName` field type is `number`, contains only the records with the `fieldName` value between `minVal` and `maxVal`. 
    * <br>2. If the `fieldName` field type is `string`, contains only the records with `fieldName` equal to `minVal`.
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
    * @param {function} callback - The filter function. It takes one parameter:
    * <br>1. `rec` - The record in the record set. Type {@link module:qm.Record}.
    * <br> Returns a `boolean` value.
    * @returns {module:qm.RecordSet} Self. Contains only the records that pass the callback function.
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
    * @param {function} callback - The splitter function. It takes two parameters:
    * <br>1. `rec` - The first record. Type {@link module:qm.Record}.
    * <br>2. `rec2` - The second record. Type {@link module:qm.Record}.
    * <br> Returns a `boolean` value.
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
    * Deletes the records, that are also in the second record set.
    * @param {module:qm.RecordSet} rs - The second record set.
    * @returns {module:qm.RecordSet} Self. Contains only the records, that are not in `rs`.
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
    //# exports.RecordSet.prototype.toJSON = function () { return {}; };
    JsDeclareFunction(toJSON);

    /**
    * Executes a function on each record in record set.
    * @param {function} callback - Function to be executed. It takes two parameters:
    * <br>1. `rec` - The current record. Type {@link module:qm.Record}.
    * <br>2. `idx` - The index of the current record (<i>optional</i>). Type `number`.
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
    * <br>1. `rec` - The current record. Type {@link module:qm.Record}.
    * <br>2. `idx` - The index of the current record (<i>optional</i>). Type `number`.
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
    * @param {module:qm.RecordSet} rs - The second record set.
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
    * @param {string} fieldName - The field from which to take the values. It's type must be one-dimensional, e.g.  `int`, `float`, `string`...
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
    * @param {string} fieldName - The field from which to take the values. It's type must be numeric, e.g. `int`, `float`, `float_v`, `num_sp_v`...
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
    * Returns the store, where the records in the record set are stored. Type {@link module:qm.Store}.
    */
    //# exports.RecordSet.prototype.store = Object.create(require('qminer').Store.prototype);
    JsDeclareFunction(store);

    /**
    * Returns the number of records in record set. Type `number`.
    */
    //# exports.RecordSet.prototype.length = 0;
    JsDeclareProperty(length);

    /**
    * Checks if the record set is empty. If the record set is empty, then it returns true. Otherwise, it returns false. Type `boolean`.
    */
    //# exports.RecordSet.prototype.empty = true;
    JsDeclareProperty(empty);

    /**
    * Checks if the record set is weighted. If the record set is weighted, then it returns true. Otherwise, it returns false. Type `boolean`.
    */
    //# exports.RecordSet.prototype.weighted = true;
    JsDeclareProperty(weighted);

    //!- `rec = rs[n]` -- return n-th record from the record set
    JsDeclIndexedProperty(indexId);
};

///////////////////////////////
// NodeJs QMiner Store Iterator
/**
* Store Iterators allows you to iterate through the records in the store. <br>
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
    ~TNodeJsStoreIter() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; RecObj.Reset(); }
    
    /**
    * Moves to the next record.
    * @returns {boolean} 
    * <br>1. `True`, if the iteration successfully moves to the next record.
    * <br>2. `False`, if there is no record left.
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
    * Gives the store of the iterator. Type {@link module:qm.Store}.
    */
    //# exports.Iterator.prototype.store = Object.create(require('qminer').Store.prototype);
    JsDeclareFunction(store);

    /**
    * Gives the current record. Type {@link module:qm.Record}.
    */
    //# exports.Iterator.prototype.record = Object.create(require('qminer').Record.prototype);
    JsDeclareFunction(record);
};

///////////////////////////////
// NodeJs QMiner record filter with JavaScript callback
class TJsRecFilter: public TQm::TRecFilter {
private:
    /// Store we are filtering
    TWPt<TQm::TStore> Store;
    /// JS Callback
    v8::Persistent<v8::Function> Callback;

public:
    TJsRecFilter(TWPt<TQm::TStore> _Store, v8::Handle<v8::Function> _Callback);
    /// We need to clean reference to callback
    ~TJsRecFilter() { Callback.Reset(); }
    /// Filter function
    bool Filter(const TQm::TRec& Rec) const;
};

///////////////////////////////
// NodeJs QMiner record splitter or comparator with JavaScript callback
class TJsRecPairFilter {
private:
    /// Store we are comparing
    TWPt<TQm::TStore> Store;
    /// JS Callback
    v8::Persistent<v8::Function> Callback;
    
public:
    TJsRecPairFilter(TWPt<TQm::TStore> _Store, v8::Handle<v8::Function> _Callback);
    /// We need to clean reference to callback
    ~TJsRecPairFilter() { Callback.Reset(); }
    /// Comparator
    bool operator()(const TUInt64IntKd& RecIdWgt1, const TUInt64IntKd& RecIdWgt2) const;
};

///////////////////////////////
// NodeJs QMiner Index Key
class TNodeJsIndexKey: public node::ObjectWrap {
    friend class TNodeJsUtil;
private:
    // Node framework
    static v8::Persistent<v8::Function> Constructor;
    ~TNodeJsIndexKey() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; }
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
* Feature extractor types. Used for constructing {@link module:qm.FeatureSpace} objects.
* @typedef {Object} FeatureExtractor
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
* The feature extractor of type `'contant'`. Used for constructing {@link module:qm.FeatureSpace} objects.
* @property {string} type - The type of the extractor. <b>Important</b>: It must be equal `'constant'`.
* @property {number} [const = 1.0] - A constant number.
* @property {string} source - The store name.
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
* The feature extractor of type `'random'`. Used for constructing {@link module:qm.FeatureSpace} objects.
* @property {string} type - The type of the extractor. <b>Important</b>: It must be equal `'random'`.
* @property {number} [seed = 0] - The seed number used to construct the random number.
* @property {string} source - The store name.
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
* The feature extractor of type `'numeric'`. Used for constructing {@link module:qm.FeatureSpace} objects.
* @property {string} type - The type of the extractor. <b>Important</b>: It must be equal `'numeric'`.
* @property {boolean} [normalize = 'false'] - Normalize values between 0.0 and 1.0.
* @property {number} [min] - The minimal value used to form the normalization.
* @property {number} [max] - The maximal value used to form the normalization.
* @property {string} field - The name of the field from which to take the value.
* @property {string} source - The store name.
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
* The feature extractor of type `'num_sp_v'`. Used for constructing {@link module:qm.FeatureSpace} objects.
* @property {string} type - The type of the extractor. <b>Important</b>: It must be equal `'num_sp_v'`.
* @property {number} [dimension = 0] - Dimensionality of sparse vectors.
* @property {boolean} [normalize = false] - Normalize vectors to L2 norm of 1.0.
* @property {string} field - The name of the field from which to take the value.
* @property {string} source - The store name.
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
* The feature extractor of type `'categorical'`. Used for constructing {@link module:qm.FeatureSpace} objects.
* @property {string} type - The type of the extractor. <b>Important</b>: It must be equal `'categorical'`.
* @property {Array.<Object>} [values] - A fixed set of values, which form a fixed feature set. No dimensionality changes if new values are seen in the upgrades.
* @property {number} [hashDimension] - A hashing code to set the fixed dimensionality. All values are hashed and divided modulo hasDimension to get the corresponding dimension.
* @property {string} field - The name of the field form which to take the values.
* @property {string} source - The store name.
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
* // create a feature space containing the categorical extractor, where it's values
* // are taken from the field "StudyGroup": "A", "B", "C" and "D"
* var ftr = new qm.FeatureSpace(base, { type: "categorical", source: "Class", field: "StudyGroup" });
* base.close();
*/

/**
* @typedef {Object} FeatureExtractorMultinomial
* The feature extractor of type `'multinomial'`. Used for constructing {@link module:qm.FeatureSpace} objects.
* @property {string} type - The type of the extractor. <b>Important</b>: It must be equal `'multinomial'`.
* @property {boolean} [normalize = 'false'] - Normalize the resulting vector of the extractor to have L2 norm 1.0.
* @property {Array.<Object>} [values] - A fixed set of values, which form a fixed feature set, no dimensionality changes if new values are seen in the updates. Cannot be used the same time as datetime.
* @property {number} [hashDimension] - A hashing code to set the fixed dimensionality. All values are hashed and divided modulo hashDimension to get the corresponding dimension.
* @property {Object} [datetime = false] - Same as `'values'`, only with predefined values which are extracted from date and time (`month`, `day of month`, `day of week`, `time of day`, `hour`).
* <br> This fixes the dimensionality of feature extractor at the start, making it not dimension as new dates are seen. Cannot be used the same time as values.
* @property {(string|Array.<String>)} field - The name of the field from which to take the key value.
* @property {(string|Array.<String>)} [valueField] - The name of the field from which to take the numeric value. 
* <br> Defaults to 1.0 for non-zero elements in the vector.
* @property {string} source - The store name.
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
* The feature extractor of type `'text'`. Used for constructing {@link module:qm.FeatureSpace} objects.
* @property {string} type - The type of the extractor. <b>Important</b>: It must be equal `'text'`.
* @property {boolean} [normalize = 'true'] - Normalize the resulting vector of the extractor to have L2 norm 1.0.
* @property {string} [weight = 'tfidf'] - Type of weighting used for scoring terms. Possible options are:
* <br>1. `'none'` - Sets 1 if the term occurs and 0 otherwise.
* <br>2. `'tf'` - Sets the term frequency in the document.
* <br>3. `'idf'` - Sets the inverse document frequency in the document.
* <br>4. `'tfidf'` - Sets the product of the `tf` and `idf` frequency.
* @property {number} [hashDimension] - A hashing code to set the fixed dimensionality. All values are hashed and divided modulo hashDimension to get the corresponding dimension.
* @property {string} field - The name of the field from which to take the value.
* @property {module:qm~FeatureTokenizer} tokenizer - The settings for extraction of text.
* @property {string} [mode] - How are multi-record cases combined into single vector. Possible options: 
* <br>1. `'concatenate'` - Multi-record cases are merged into one document.
* <br>2. `'centroid'` - Treat each case as a seperate document.
* <br>3. `'tokenized'` - use the tokenizer option.
* @property {string} [stream] - Details on forgetting old IDFs when running on stream. Possible options:
* <br>1. `'field'` - Field name which is providing timestamp. If missing, system time is used (<i>optional</i>).
* <br>2. `'factor'` - Forgetting factor, by which the olf IDFs are multiplied after each iteration.
* <br>3. `'interval'` - The time between iterations when the factor is applied, standard JSON time format is used to specify the interval duration.
* @property {string} source - The store name.
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
* The feature extractor of type `'join'`. Used for constructing {@link module:qm.FeatureSpace} objects.
* @property {string} type - The type of the extractor. <b>Important</b>: It must be equal `'join'`.
* @property {number} [bucketSize = 1] - The size of the bucket in which we group consecutive records.
* @property {string} source - The store name.
* @example
* // import qm module
* var qm = require('qminer');
*/

/**
* @typedef {Object} FeatureExtractorPair
* The feature extractor of type `'pair'`. Used for constructing {@link module:qm.FeatureSpace} objects.
* @property {string} type - The type of the extractor. <b>Important</b>: It must be equal `'pair'`.
* @property {module:qm~FeatureExtractors} first - The first feature extractor.
* @property {module:qm~FeatureExtractors} second - The second feature extractor.
* @property {source} source - The store name.
* @example
* var qm = require('qminer');
*/

/**
* @typedef {Object} FeatureExtractorDateWindow
* The feature extractor of type `'dateWindow'`. Used for constructing {@link module:qm.FeatureSpace} objects.
* @property {string} type - The type of the extractor. <b>Important</b>: It must be equal `'dateWindow'`.
* @property {string} [unit = 'day'] - How granular is the time window. Possible options are `'day'`, `'week'`, `'month'`, `'year'`, `'12hours'`, `'6hours'`, `'4hours'`, `'2hours'`,
* `'hour'`, `'30minutes'`, `'15minutes'`, `'10minutes'`, `'minute'`, `'second'`.
* @property {number} [window = 1] - The size of the window.
* @property {boolean} [normalize = 'false'] - Normalize the resulting vector of the extractor to have L2 norm 1.0. //TODO
* @property {number} start - //TODO
* @property {number} end - //TODO
* @property {string} source - The store name.
* @example
* // import qm module
* var qm = require('qminer');
*/

/**
* @typedef {Object} FeatureExtractorJsfunc
* The feature extractor of type `'jsfunc'`. Used for constructing {@link module:qm.FeatureSpace} objects.
* @property {string} type - The type of the extractor. <b>Important</b>: It must be equal `'jsfunc'`.
* @property {string} name - The features name.
* @property {function} fun - The javascript function callback. It takes one parameter: 
* <br>1. `rec` - The record. Type {@link module:qm.Record}.
* It returns `number` or {@link module:la.Vector}.
* @property {number} [dim = 1] - The dimension of the feature extractor.
* @property {string} source - The store name.
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
* The settings for extraction of text used in {@link module:qm~FeatureExtractorText}.
* @typedef {Object} FeatureTokenizer
* @property {string} [type = 'simple'] - The type of the encoding text. Possible options:
* <br>1. `'simple'` - The simple encoding.
* <br>2. `'html'` - The html encoding.
* <br>3.²`'unicode'` - The unicode encoding.
* @property {string | Array.<string>} [stopwords = 'en'] - The stopwords used for extraction. Possible options:
* <br>1. `'none'` - No pre-defined stopword list. Type `string`.
* <br>2. `'en'` - The english pre-defined stopword list. Type `string`.
* <br>3. `'si'` - The slovene pre-defined stopword list. Type `string`.
* <br>4. `'es'` - The spanish pre-defined stopword list. Type `string`.
* <br>5. `'de'` - The german pre-defined stopword list. Type `string`.
* <br>6. `array` - An array of stopwords. The array must be given as a parameter. Type `Array of strings`.
* @property {string} [stemmer = 'none'] - The stemmer used for extraction. Possible options:
* <br>1. `'true'` - Using the porter stemmer. 
* <br>2. `'porter'` - Using the porter stemmer. 
* <br>3. `'none'` - Using no stemmer. 
* @property {boolean} [uppercase = 'true'] - Changing all words to uppercase.
*/

/////////////////////////////
// Feature Space Desc

/**
* Feature Space
* @classdesc Represents the feature space. It contains any of the {@link module:qm~FeatureExtractor} objects.
* @class
* @param {module:qm.Base} base - The base where the features are extracted from.
* @param {(Array.<module:qm~FeatureExtractor> | module:fs.FIn)} arg - Constructor arguments. There are two ways of constructing:
* <br>1. Using an array of {@link module:qm~FeatureExtractor} objects, 
* <br>2. using a file input stream {@link module:fs.FIn}.
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
//# exports.FeatureSpace = function (base, arg) { return Object.create(require('qminer').FeatureSpace.prototype); };

class TNodeJsFtrSpace : public node::ObjectWrap {
    friend class TNodeJsUtil;
private:
    static v8::Persistent<v8::Function> Constructor;
    ~TNodeJsFtrSpace() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; }
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
    * Returns the dimension of the feature space. Type `number`.
    */
    //# exports.FeatureSpace.prototype.dim = 0;
    JsDeclareProperty(dim);
    
    /**
    * Returns an array of the dimensions of each feature extractor in the feature space. Type `Array of numbers`.
    */
    //# exports.FeatureSpace.prototype.dims = [0];
    JsDeclareProperty(dims);

    /**
    * Serialize the feature space to an output stream.
    * @param {module:fs.FOut} fout - The output stream.
    * @returns {module:fs.FOut} The output stream `fout`.
    */
    //# exports.FeatureSpace.prototype.save = function (fout) { return Object.create(require('qminer').fs.FOut.prototype); };
    JsDeclareFunction(save);

    /**
    * Clears the feature space.
    * @returns {module:qm.FeatureSpace} Self. Features space has been cleared.
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
    * @param {module:qm~FeatureExtractor} ftExt - The added feature extractor.
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
    //# exports.FeatureSpace.prototype.addFeatureExtractor = function (ftExt) { return Object.create(require('qminer').FeatureSpace.prototype); };
    JsDeclareFunction(addFeatureExtractor);

    /**
    * Updates the feature space definitions and extractors by adding one record.
    * <br> For text feature extractors, it can update it's vocabulary by taking into account the new text.
    * <br> For numeric feature extractors, it can update the minimal and maximal values used to form the normalization.
    * <br> For jsfunc feature extractors, it can update a parameter used in it's function.
    * <br> For dateWindow feature extractor, it can update the start and the end of the window period to form the normalization.
    * @param {module:qm.Record} rec - The record, which updates the feature space.
    * @returns {module:qm.FeatureSpace} Self. The feature space has been updated.
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
    * @returns {module:qm.FeatureSpace} Self. The feature space has been updated.
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
    * @param {number} [idx] - When given, only use specified feature extractor.
    * @returns {module:la.SparseVector} The sparse feature vector gained from `rec` and `idx`.
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
    //# exports.FeatureSpace.prototype.extractSparseVector = function (rec, idx) { return Object.create(require('qminer').la.SparseVector.prototype); }
    JsDeclareFunction(extractSparseVector);

    /**
    * Creates a feature vector from the given record.
    * @param {module:qm.Record} rec - The given record.
    * @param {number} [idx] - when given, only use specified feature extractor.
    * @returns {module:la.Vector} The feature vector gained from `rec` and `idx`.
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
    //# exports.FeatureSpace.prototype.extractVector = function (rec, idx) { return Object.create(require('qminer').la.Vector.prototype); };
    JsDeclareFunction(extractVector);

    /**
     * Extracts a single feature using the feature extractor.
     * @param {Integer} idx - Index of the feature extractor.
     * @param {Number} val - Value to extract.
     * @returns {object} The extracted single feature.
     */
    //# exports.FeatureSpace.prototyp.extractFeature = function (idx, val) { }
    JsDeclareFunction(extractFeature);
    
    /**
    * Extracts the sparse feature vectors from the record set and returns them as columns of the sparse matrix.
    * @param {module:qm.RecordSet} rs - The given record set.
    * @param {number} [idx] - When given, only use specified feature extractor.
    * @returns {module:la.SparseMatrix} The sparse matrix, where the i-th column is the sparse feature vector of the i-th record in `rs`.
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
    //# exports.FeatureSpace.prototype.extractSparseMatrix = function (rs, idx) { return Object.create(require('qminer').la.SparseMatrix.prototype); };
    JsDeclareFunction(extractSparseMatrix);

    /**
    * Extracts the feature vectors from the recordset and returns them as columns of a dense matrix.
    * @param {module:qm.RecordSet} rs - The given record set.
    * @param {number} [idx] - when given, only use specified feature extractor.
    * @returns {module:la.Matrix} The dense matrix, where the i-th column is the feature vector of the i-th record in `rs`.
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
    * // 0  1  1  1
    * // 0  0  1  0
    * // 1  1  0  0
    * var matrix = ftr.extractMatrix(base.store("Class").allRecords);
    * base.close();
    */
    //# exports.FeatureSpace.prototype.extractMatrix = function (rs, idx) { return Object.create(require('qminer').la.Matrix.prototype); };
    JsDeclareFunction(extractMatrix);

    JsDeclareAsyncFunction(extractMatrixAsync, TExtractMatrixTask);

    /**
    * Gives the name of feature extractor at given position.
    * @param {number} idx - The index of the feature extractor in feature space (zero based).
    * @returns {String} The name of the feature extractor at position `idx`.
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
    JsDeclareFunction(getFeatureExtractorType);

    /**
    * Gives the name of the feature at the given position.
    * @param {number} idx - The index of the feature in feature space (zero based).
    * @returns {String} The name of the feature at the position `idx`.
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
    * Performs the inverse operation of `ftrVec`. Works only for numeric feature extractors.
    * @param {(module:la.Vector | Array.<Object>)} ftr - The feature vector or an array with feature values.
    * @returns {Array.<Object>} The inverse of `ftr` as an array.
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
    * @returns {Object} The inverse of `val` using the feature extractor with index `idx`.
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

    JsDeclareFunction(getFeatureRange);

    /**
    * Filters the vector to keep only the elements from the feature extractor.
    * @param {(module:la.Vector | module:la.SparseVector)} vec - The vector from where the function filters the elements.
    * @param {number} idx - The index of the feature extractor.
    * @param {boolean} [keepOffset = 'true'] - For keeping the original indexing in the new vector.
    * @returns {(module:la.Vector | module:la.SparseVector)} 
    * <br>1. {@link module:la.Vector}, if `vec` is {@link module:la.Vector}.
    * <br>2. {@link module:la.SparseVector}, if `vec` is {@link module:la.SparseVector}.
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
    * @param {module:qm.Record} rec - The record.
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

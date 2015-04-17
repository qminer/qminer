//intellisense start
// this file mimicks the qminer module index.js file
exports = {}; require.modules.qminer = exports;
exports.la = require('qminer_la');
exports.fs = require('qminer_fs');
exports.analytics = require('qminer_analytics');
exports.ht= require('qminer_ht');
exports.statistics= require('qminer_stat');
exports.datasets= require('qminer_datasets');
//intellisense end
/**
* Qminer module.
* @module qm
* @example 
* // import module
* var qm = require('qminer');
*/
/**
	* Creates a directory structure.
	* @param {string} [configPath='qm.conf'] - The path to configuration file.
	* @param {boolean} [overwrite=false] - If you want to overwrite the configuration file.
	* @param {number} [portN=8080] - The number of the port. Currently not used.
	* @param {number} [cacheSize=1024] - Sets available memory for indexing (in MB).
	*/
 exports.config = function(configPath, overwrite, portN, cacheSize) {}
/**
	* Creates an empty base.
	* @param {string} [configPath='qm.conf'] - Configuration file path.
	* @param {string} [schemaPath=''] - Schema file path.
	* @param {boolean} [clear=false] - Clear the existing db folder.
	* @returns {module:qm.Base}
	*/
 exports.create = function (configPath, schemaPath, clear) { return Object.create(require('qminer').Base.prototype); }
/**
	* Opens a base.
	* @param {string} [configPath='qm.conf'] - The configuration file path.
	* @param {boolean} [readOnly=false] - Open in read only mode?
	* @returns {module:qm.Base}
	*/
 exports.open = function (configPath, readOnly) { return Object.create(require('qminer').Base.prototype); }
/**
* Base access modes.
* @readonly
* @enum {string}
*/
 var baseModes = {
    /** sets up the db folder */
    create: 'create',
    /** cleans the db folder and calls create */
    createClean: 'createClean',
    /** opens with write permissions */
    open: 'open',
    /** opens in read-only mode */
    openReadOnly: 'openReadOnly'
 }
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
* @property  {Array<module:qm~SchemaDefinition>} [BaseConstructorParam.schema=[]] - Schema definition object array.
* @property  {string} [BaseConstructorParam.dbPath='./db/'] - The path to db directory.
*/
/**
* Store schema definition object
* @typedef {Object} SchemaDefinition
* @property {string} name - The name of the store. Store name can be composed by from English letters, numbers, _ or $ characters. It can only begin with a character.
* @property {Array<module:qm~SchemaFieldDefinition>} fields - The array of field descriptors. 
* @property {Array<module:qm~SchemaJoinDefinition>} [joins=[]] - The array of join descriptors, used for linking records from different stores.
* @property {Array<module:qm~SchemaKeyDefinition>} [keys=[]] - The array of key descriptors. Keys define how records are indexed, which is needed for search using the query language.
* @property {module:qm~SchemaTimeWindowDefinition} [timeWindow] - Time window description. Stores can have a window, which is used by garbage collector to delete records once they fall out of the time window. Window can be defined by number of records or by time.
* @example
* var qm = require('qminer');
* // create a simple movies store, where each record contains only the movie title.
* var base = new qm.Base({
*     mode: 'createClean',
*     schema: [{
*       "name": "Movies",
*       "fields": [{ name: "title", type: "string" }]
*     }]
* });
*/
/**
* Field types.
* @readonly
* @enum {string}
*/
 var fieldTypes = {
    /** signed 32-bit integer */
    int: 'int', 
    /** vector of signed 32-bit integers */
    int_v: 'int_v', 
 /** string */
 string : 'string',
 /** vector of strings */
 string_v : 'string_v',
 /** boolean */
 bool : 'bool',
 /** double precision floating point number */
 float : 'float',
 /** a pair of floats, useful for storing geo coordinates */
 float_pair : 'float_pair',
 /** vector of floats */
 float_v : 'float_v',
 /** date and time format, stored in a form of milliseconds since 1600 */
 datetime : 'datetime',
 /** sparse vector(same format as used by QMiner JavaScript linear algebra library) */
 num_sp_v : 'num_sp_v',
 }
/**
* Store schema field definition object
* @typedef {Object} SchemaFieldDefinition
* @property {string} name - The name of the field.
* @property {module:qm~fieldTypes} type - The type of the field.
* @property {boolean} [primary=false] - Field which can be used to identify record. There can be only one primary field in a store. There can be at most one record for each value of the primary field. Currently following fields can be marked as primary: int, uin64, string, float, datetime. Primary fields of type string are also used for record names.
* @property {boolean} [null=false] - When set to true, null is a possible value for a field (allow missing values).
* @property {string} [store='memory'] - Defines where to store the field, options are: <b>'cache'</b> or <b>'memory'</b>. The default option is <b>'memory'</b>, which stores the values in RAM. Option <b>'cache'</b> stores the values on disk, with a layer of FIFO cache in RAM, storing the most recently used values.
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
* base.store('NewsArticles').add({
*   ID: 't12344', 
*   Source: 's1234', 
*   DateTime: '2015-01-01T00:05:00', 
*   Title: 'the title', 
*   Tokens: ['token1', 'token2'], 
*   Vector: [[0,1], [1,1]]})
*/
/**
* Store schema join definition object
* @typedef {Object} SchemaJoinDefinition
* @property {string} name - The name of the join.
* @property {string} type - The supported types are: <b>'field'</b> and <b>'index'</b>. 
* <br> A join with type=<b>'field'</b> can point to zero or one record and is implemented as an additional hidden field of type uint64, which can hold the ID of the record it links to. Accessing the record's join returns a record.
* <br> A join with type=<b>'index'</b> can point to any number of records and is implemented using the inverted index, where for each record a list (vector) of linked records is kept. Accessing the record's join returns a record set.
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
* base.store('Movies').add({ title: 'Broken Flowers', director: { name: 'Jim Jarmusch' } });
* // Adds a movie, sets the 'director' join, updates the index join of 'Jim Jarmusch'
* base.store('Movies').add({ title: 'Coffee and Cigarettes', director: { name: 'Jim Jarmusch' } });
* // Adds movie, automatically adds 'Lars von Trier' to People, sets the 'director' join
* // and 'directed' inverse join (automatically)
* base.store('Movies').add({ title: 'Dogville', director: { name: 'Lars von Trier' } });
*
* var movie = base.store('Movies')[0]; // get the first movie (Broken Flowers)
* // Each movie has a property corresponding to the join name: 'director'. 
* // Accessing the property returns a {@link module:qm.Record} from the store People.
* var person = movie.director; // get the director
* console.log(person.name); // prints 'Jim Jarmusch'
* // Each person has a property corresponding to the join name: 'directed'. 
* // Accessing the property returns a {@link module:qm.RecSet} from the store People.
* var movies = person.directed; // get all the movies the person directed.
* movies.each(function (movie) { console.log(movie.title); }); 
* // prints: 
* //   'Broken Flowers'
* //   'Coffee and Cigarettes'
*/
/**
* Store schema key definition object
* @typedef {Object} SchemaKeyDefinition
* @property {string} field - The name of the field that will be indexed.
* @property {string} type - The supported types are: <b>'value'</b>, <b>'text'</b> and <b>'location'</b>.
* <br> A key with type=<b>'value'</b> indexes records using an inverted index using full value of the field (no processing).
*  The key type supports 'string', 'string_v' and 'datetime' fields types.
* <br> A key with type=<b>'text'</b> indexes string fields by using a tokenizer and text processing. Supported by string fields.
* <br> A key with type=<b>'location'</b> indexes records as points on a sphere and enables nearest-neighbour queries. Supported by float_pair type fields.
* @property {string} [name] - Allows using a different name for the key in search queries. This allows for multiple keys to be put against the same field. Default value is the name of the field.
* @property {string} [vocabulary] - defines the name of the vocabulary used to store the tokens or values. This can be used indicate to several keys to use the same vocabulary, to save on memory. Supported by 'value' and 'text' keys.
* @property {string} [tokenize] - defines the tokenizer that is used for tokenizing the values stored in indexed fields. Tokenizer uses same parameters as in bag-of-words feature extractor. Default is english stopword list and no stemmer. Supported by 'text' keys.
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
* base.store('People').add({name : 'John Smith'});
* base.store('People').add({name : 'Mary Smith'});
* // search based on indexed values
* base.search({$from : 'People', name: 'John Smith'}); // Return the record set containing 'John Smith'
* // search based on indexed values
* base.search({$from : 'People', name: 'Smith'}); // Returns the empty record set.
* // search based on text indexing
* base.search({$from : 'People', nameText: 'Smith'}); // Returns both records.
*/
/**
* Stores can have a window, which is used by garbage collector to delete records once they
* fall out of the time window. Window can be defined by number of records or by time.
* Window defined by parameter window, its value being the number of records to be kept.
* @typedef {Object} SchemaTimeWindowDefinition
* @property {number} duration - the size of the time window (in number of units).
* @property {string} unit - defines in which units the window size is specified. Possible values are <b>second</b>, <b>minute</b>, <b>hour</b>, <b>day</b>, <b>week</b> or <b>month</b>.
* @property {string} [field] - name of the datetime filed, which defines the time of the record. In case it is not given, the insert time is used in its place.
* @example
* var qm = require('qminer');
* // Create a store
* var base = new qm.Base([{
* // ...
*   timeWindow : { 
*     duration : 12,
*     unit : "hour",
*     field : "DateTime"
*   }
* }]);
*/
/**
* Feature types.
* @typedef {Object} FeatureTypes
* @property {module:qm~FeatureTypeConstant} constant - The constant type.
* @property {module:qm~FeatureTypeRandom} random - The random type.
* @property {module:qm~FeatureTypeNumeric} numeric - The numeric type.
* @property {module:qm~FeatureTypeCategorical} categorical - The categorical type.
* @property {module:qm~FeatureTypeMultinomial} multinomial - The multinomial type.
* @property {module:qm~FeatureTypeText} text - The text type.
* @property {module:qm~FeatureTypeJoin} join - The join type.
* @property {module:qm~FeatureTypePair} pair - The pair type.
* @property {module:qm~FeatureTypeJsfunc} jsfunc - The jsfunc type.
* @property {module:qm~FeatureTypeDateWindow} dateWindow - The dateWindow type.
*
*/
/**
* Feature type: contant
* @typedef {Object} FeatureTypeConstant
* @property {number} [const = 1.0] - A constant number. 
*/
/**
* Feature type: random
* @typedef {Object} FeatureTypeRandom
* @property {number} [seed = 0] - A random seed number.
*/
/**
* Feature type: numeric
* @typedef {Object} FeatureTypeNumeric 
* @property {boolean} [normalize = false] - Normalize values between 0.0 and 1.0.
* @property {number} [min] - The minimal value used to form the normalization.
* @property {number} [max] - The maximal value used to form the normalization.
* @property {string} field - The name of the field from which to take the value.
*/
/**
* Feature type: categorical
* @typedef {Object} FeatureTypeCategorical
* @property {Array.<Object>} [values] - A fixed set of values, which form a fixed feature set. No dimensionalizy changes if new values are seen in the upgrades.
* @property {number} [hashDimension] - A hashing code to set the fixed dimensionality. All values are hashed and divided modulo hasDimension to get the corresponding dimension.
* @property {string} field - The name of the field form which to take the values.
*/
/**
* Feature extractor parameter object
* @typedef {Object} FeatureExtractor
* @property {module:qm~FeatureTypes} type - The type of the extractor.
* @property {module:qm~FeatureSource} source - The source of the extractor.
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
 exports.Base = function (paramObj) {};
/**
	* Closes the database.
	* @returns {null}
	*/
 exports.Base.prototype.close = function () {}
/**
	 * Returns the store with the specified name.
	 *
	 * @param {string} name - Name of the store.
	 * @returns {module:qm.Store} The store.
	 */
 exports.Base.prototype.store = function (name) { return Object.create(require('qminer').Store.prototype); }
/**
	 * Returns a list of store descriptors.
	 *
	 * @returns {Object[]}
	 */
 exports.Base.prototype.getStoreList = function () { return [{storeId:'', storeName:'', storeRecords:'', fields: [], keys: [], joins: []}]; }
/**
	* Creates a new store.
	* @param {Array<module:qm~SchemaDefinition>} storeDef - The definition of the store(s)
	* @param {number} [storeSizeInMB = 1024] - The reserved size of the store(s).
	* @returns {(module:qm.Store | module:qm.Store[])} - Returns a store or an array of stores (if the schema definition was an array)
	*/
 exports.Base.prototype.createStore = function (storeDef, storeSizeInMB) { return storeDef instanceof Array ? [Object.create(require('qminer').Store.prototype)] : Object.create(require('qminer').Store.prototype) ;}
/**
	* Creates a new store.
	* @param {module:qm~QueryObject} query - query language JSON object	
	* @returns {module:qm.RecSet} - Returns the record set that matches the search criterion
	*/
 exports.Base.prototype.search = function (query) { return Object.create(require('qminer').RecSet.prototype);}
/**
	* Calls qminer garbage collector to remove records outside time windows.
	*/
 exports.Base.prototype.gc = function () { }
/**
* Store (factory pattern result) 
* @namespace
* @example
* // import qm module
* var qm = require('qminer');
* // factory based construction using base.createStore
* // TODO
* // using a constructor
* // TODO
*/
 exports.Store = function (base, storeDef) {};
/**
	* Returns a record form the store.
	* @param {string} recName - Record name.
	* @returns {Object} Returns the record. If record doesn't exist, it returns null. //TODO
	*/
 exports.Store.prototype.rec = function (recName) {};
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
	* store.each(function (rec) { rec.Gender = "Extraterrestrial"; });
	*/
 exports.Store.prototype.each = function (callback) {}
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
	* // make an array of record names
	* var arr = store.map(function (rec) { return rec.Name; });
	*/
 exports.Store.prototype.map = function (callback) {}
/**
	* Adds a record to the store.
	* @param {Object} rec - The added record. //TODO
	* @returns {number} The ID of the added record.
	*/
 exports.Store.prototype.add = function (rec) {}
/**
	* Creates a new record of given store. The record is not added to the store.
	* @param {Object} json - A JSON value of the record.
	* @returns {module:qm.Record} The record created by the JSON value and the store.
	*/
 exports.Store.prototype.newRec = function (json) {};
/**
	* Creates a new record set out of the records in store.
	* @param {module:la.IntVector} idVec - The integer vector containing the ids of selected vectors.
	* @returns {module:qm.RecSet} The record set that contains the records gained with idVec.
	*/
 exports.Store.prototype.newRecSet = function (idVec) {};
/**
	* Creates a record set containing random records from store.
	* @param {number} sampleSize - The size of the record set.
	* @returns {module:qm.RecSet} Returns a record set containing random records.
	*/
 exports.Store.prototype.sample = function (sampleSize) {};
/**
	* Gets the details of the selected field.
	* @param {string} fieldName - The name of the field.
	* @returns {Object} The JSON object containing the details of the field. //TODO
	*/
 exports.Store.prototype.field = function (fieldName) {}; 
/**
	* Checks if the field is of numeric type.
	* @param {string} fieldName - The checked field.
	* @returns {boolean} True, if the field is of numeric type. Otherwise, false.
	*/
 exports.Store.prototype.isNumeric = function (fieldName) {};
/**
	* Checks if the field is of string type.
	* @param {string} fieldName - The checked field.
	* @returns {boolean} True, if the field is of the string type. Otherwise, false.
	*/
 exports.Store.prototype.isString = function (fieldName) {}; 
/**
	* Returns the details of the selected key.
	* @param {string} keyName - The selected key.
	* @returns {Object} The JSON object containing the details of the key. //TODO
	*/
 exports.Store.prototype.key = function (keyName) {};
/**
	* Returns the store as a JSON.
	* @returns {Object} The store as a JSON.
	*/
 exports.Store.prototype.toJSON = function () {};
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
 exports.Store.prototype.clear = function (num) {};
/**
	* Gives a vector containing the field value of each record.
	* @param {string} fieldName - The field name. Field must be of one-dimensional type, e.g. int, float, string...
	* @returns {module:la.Vector} The vector containing the field values of each record.
	*/
 exports.Store.prototype.getVec = function (fieldName) {};
/**
	* Gives a matrix containing the field values of each record.
	* @param {string} fieldName - The field name. Field mustn't be of type string.
	* @returns {(module:la.Matrix | module:la.SparseMatrix)} The matrix containing the field values. 
	*/
 exports.Store.prototype.getMat = function (fieldName) {};
/**
	* Gives the field value of a specific record.
	* @param {number} recId - The record id.
	* @param {string} fieldName - The field's name.
	* @returns {Object} The fieldName value of the record with recId.
	*/
 exports.Store.prototype.cell = function (recId, fieldName) {};
/**
	* Gives the name of the store.
	*/
 exports.Store.prototype.name = undefined;
/**
	* Checks if the store is empty.
	*/
 exports.Store.prototype.empty = undefined;
/**
	* Gives the number of records.
	*/
 exports.Store.prototype.length = 0;
/**
	* Creates a record set containing all the records from the store.
	*/
 exports.Store.prototype.recs = undefined;
/**
	* Gives an array of all field descriptor JSON objects.
	*/
 exports.Store.prototype.fields = undefinied;
/**
	* Gives an array of all join descriptor JSON objects.
	*/
 exports.Store.prototype.joins = undefined;
/**
	* Gives an array of all key descriptor JSON objects.
	*/
 exports.Store.prototype.keys = undefined;
/**
	* Returns the first record of the store.
	*/
 exports.Store.prototype.first = undefined;
/**
	* Returns the last record of the store.
	*/
 exports.Store.prototype.last = undefined;
/**
	* Returns an iterator for iterating over the store from start to end.
	*/
 exports.Store.prototype.forwardIter = undefined;
/**
	* Returns an iterator for iterating over the store form end to start.
	*/
 exports.Store.prototype.backwardIter = undefined;
/**
* Record (factory pattern).
* @namespace
*/
 exports.Record = function () {}; 
/**
	* Clones the record.
	* @returns {module:qm.Record} The clone of the record.
	*/
 exports.Record.prototype.$clone = function () {};
/**
	* Creates a JSON version of the record.
	* @returns {Object} The JSON version of the record.
	*/
 exports.Record.prototype.toJSON = function () {};
/**
	* Returns the id of the record.
	*/
 exports.Record.prototype.$id = undefined;
/**
	* Returns the name of the record.
	*/
 exports.Record.prototype.$name = undefined;
/**
	* Returns the frequency of the record.
	*/
 exports.Record.prototype.$fq = undefined;
/**
	* Returns the store the record belongs to.
	*/
 exports.Record.prototype.store = undefined;
/**
* Record Set (factory pattern)
* @namespace
* @example
* // import qm module
* var qm = require('qminer');
* // factory based construction using store.recs
* var rs = store.recs;
*/
 exports.RecSet = function () {}
/**
	* Creates a new instance of the record set.
	* @returns {module:qm.RecSet} A copy of the record set.
	*/
 exports.RecSet.prototype.clone = function () {};
/**
	* Creates a new record set out of the join attribute of records.
	* @param {string} joinName - The name of the join attribute.
	* @param {number} [sampleSize] - The number of records to be used for construction of the record set.
	* @returns {module:qm.RecSet} The record set containing the join records.
	*/
 exports.RecSet.prototype.join = function (joinName, sampleSize) {};
/**
	* Truncates the first records.
	* @param {number} limit_num - How many records to truncate.
	* @param {number} [offset_num] - Where to start to truncate.
	* @returns {module:qm.RecSet} Self.
	* @example
	* // import qm module
	* qm = require('qminer');
	* // construct a record set with 20 records
	* rs = //TODO
	* rs2 = //TODO
	* // truncate the first 10 records
	* rs.trunc(10); // returns self, only with the first 10 records
	* // truncate the first 10 records starting with the 5th
	* rs2.trunc(10, 4);
	*/
 exports.RecSet.prototype.trunc = function (limit_num, offset_num) {};
/**
	* Creates a sample of records of the record set.
	* @param {number} num - The number of records in the sample.
	* @returns {module:qm.RecSet} A record set containing the sample records.
	*/
 exports.RecSet.prototype.sample = function (num) {};
/**
	* Shuffles the order of records in the record set.
	* @param {number} [seed] - Integer.
	* @returns {module:qm.RecSet} Self.
	*/
 exports.RecSet.prototype.shuffle = function (seed) {};
/**
	* It reverses the record order.
	* @returns {module:qm.RecSet} Self. Records are in reversed order.
	*/
 exports.RecSet.prototype.reverse = function () {};
/**
	* Sorts the records according to record id.
	* @param {number} [asc=1] - If asc > 0, it sorts in ascending order. Otherwise, it sorts in descending order.  
	* @returns {module:qm.RecSet} Self. Records are sorted according to record id and asc.
	*/
 exports.RecSet.prototype.sortById = function (asc) {}; 
/**
	* Sorts the records according to a specific record field.
	* @param {string} fieldName - The field by which the sort will work.
	* @param {number} [arc=-1] - if asc > 0, it sorts in ascending order. Otherwise, it sorts in descending order.
	* @returns {module:qm.RecSet} Self. Records are sorted according to fieldName and arc.
	*/
 exports.RecSet.prototype.sortByField = function (fieldName, asc) {};
/**
	* Sorts the records according to the given callback function.
	* @param {function} callback - The function used to sort the records. It takes two parameters:
	* <br>1. rec - The first record.
	* <br>2. rec2 - The second record.
	* <br>It returns a boolean object.
	* @returns {module:qm.RecSet} Self. The records are sorted according to the callback function.
	* @example
	* // import qm module
	* qm = require('qminer');
	* // construct a new record set of movies (one field is it's Rating)
	* var rs = //TODO
	* // sort the records by their rating
	* rs.sort(function (rec, rec2) { return rec.Rating < rec2.Rating ;});
	*/
 exports.RecSet.prototype.sort = function (callback) {};
/**
	* Keeps only records with ids between two values.
	* @param {number} [minId] - The minimum id.
	* @param {number} [maxId] - The maximum id.
	* @returns {module:qm.RecSet} Self. 
	* <br>1. Contains only the records of the original with ids between minId and maxId, if parameters are given.
	* <br>2. Contains all the records of the original, if no parameter is given.
	*/
 exports.RecSet.prototype.filterById = function (minId, maxId) {};
/**
	* Keeps only the records with a specific value of some field.
	* @param {string} fieldName - The field by which the records will be filtered.
	* @param {(string | number)} minVal -  
	* <br>1. Is a string, if the field type is a string. The exact string to compare.
	* <br>2. Is a number, if the field type is a number. The minimal value for comparison.
	* <br>3. TODO Time field
	* @param {number} maxVal - Only in combination with minVal for non-string fields. The maximal value for comparison.
	* @returns {module:qm.RecSet} Self. Containing only the records with the fieldName value between minVal and maxVal. If the fieldName type is string,
	* it contains only the records with fieldName equal to minVal.
	*/
 exports.RecSet.prototype.filterByField = function (fieldName, minVal, maxVal) {};
/**
	* Keeps only the records that pass the callback function.
	* @param {function} callback - The filter function. It takes one parameter and return a boolean object.
	* @returns {module:qm.RecSet} Self. Containing only the record that pass the callback function.
	* @example
	* // import qm module
	* qm = require('qminer');
	* // construct a record set of kitchen appliances
	* var rs = //TODO
	* // filter by the field price
	* rs.filter(function (rec) { return rec.Price > 10000; }); // keeps only the records, where their Price is more than 10000
	*/
 exports.RecSet.prototype.filter = function (callback) {}; 
/**
	* Splits the record set into smaller record sets.
	* @param {function} callback - The splitter function. It takes two parameters (records) and returns a boolean object.
	* @returns {Array.<module:qm.RecSet>} An array containing the smaller record sets. The records are split according the callback function.
	*/
 exports.RecSet.prototype.split = function (callback) {};
/**
	* Deletes the records, that are also in the other record set.
	* @param {module:qm.RecSet} rs - The other record set.
	* @returns {module:qm.RecSet} Self. Contains only the records, that are not in rs.
	*/
 exports.RecSet.prototype.deleteRecs = function (rs) {}; 
/**
	* Returns the record set as a JSON.
	* @returns {Object} The record set as a JSON.
	*/
 exports.RecSet.prototype.toJSON = function () {};
/**
	* Executes a function on each record in record set.
	* @param {function} callback - Function to be executed. It takes two parameters:
	* <br>rec - The current record.
	* <br>[idx] - The index of the current record.
	* @returns {module:qm.RecSet} Self.
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a record set with some people with fields Name and Gender
	* var rs = //TODO
	* // change the gender of all records to "Extraterrestrial"
	* rs.each(function (rec) { rec.Gender = "Extraterrestrial"; });
	*/
 exports.RecSet.prototype.each = function (callback) {}
/**
	* Creates an array of function outputs created from the records in record set.
	* @param {function} callback - Function that generates the array. It takes two parameters:
	* <br>rec - The current record.
	* <br>[idx] - The index of the current record.
	* @returns {Array<Object>} The array created by the callback function. //TODO
	* @example
	* // import qm module
	* var qm = require('qminer');
	* // create a record set with some people with fields Name and Gender
	* var rs = //TODO
	* // make an array of record names
	* var arr = rs.map(function (rec) { return rec.Name; });
	*/
 exports.RecSet.prototype.map = function (callback) {}
/**
	* Creates the set intersection of two record sets.
	* @param {module:qm.RecSet} rs - The other record set.
	* @returns {module:qm.RecSet} The intersection of the two record sets.
	*/
 exports.RecSet.prototype.setintersect = function (rs) {};
/**
	* Creates the set union of two record sets.
	* @param {module:qm.RecSet} rs - The other record set.
	* @returns {module:qm.RecSet} The union of the two record sets.
	*/
 exports.RecSet.prototype.setunion = function (rs) {};
/**
	* Creates the set difference between two record sets.
	* @param {module:qm.RecSet} rs - The other record set.
	* @returns {module:qm.RecSet} The difference between the two record sets.
	*/
 exports.RecSet.prototype.setdiff = function (rs) {}; 
/**
	* Creates a vector containing the field values of records.
	* @param {string} fieldName - The field from which to take the values. It's type must be one-dimensional, e.g. float, int, string,...
	* @returns {module:la.Vector} The vector containing the field values of records. The type it contains is dependant of the field type.
	*/
 exports.RecSet.prototype.getVec = function (fieldName) {}; 
/**
	* Creates a vector containing the field values of records.
	* @param {string} fieldName - The field from which to take the values. It's type must be numeric, e.g. float, int, float_v, num_sp_v,...
	* @returns {(module:la.Matrix|module:la.SparseMatrix)} The matrix containing the field values of records.
	*/
 exports.RecSet.prototype.getVec = function (fieldName) {};
/**
	* Returns the store, where the records in the record set are stored.
	*/
 exports.RecSet.prototype.store = undefined;
/**
	* Returns the number of records in record set.
	*/
 exports.RecSet.prototype.length = undefined;
/**
	* Checks if the record set is empty. If the record set is empty, then it returns true. Otherwise, it returns false.
	*/
 exports.RecSet.prototype.empty = undefined;
/**
	* Checks if the record set is weighted. If the record set is weighted, then it returns true. Otherwise, it returns false.
	*/
 exports.RecSet.prototype.weighted = undefined;
/**
* Store Iterator (factory pattern)
* @namespace
* @example
* // import qm module
* qm = require('qminer');
* // factory based construction with store.forwardIter
* var iter = store.forwardIter;
*/
 exports.Iterator = function () {};
/**
	* Moves to the next record.
	* @returns {boolean} 
	* <br>1. True, if the iteration successfully moves to the next record.
	* <br>2. False, if there is no record left.
	*/
 exports.Iterator.prototype.next = function () {};
/**
	* Gives the store of the iterator.
	*/
 exports.Iterator.prototype.store = undefined;
/**
	* Gives the current record.
	*/
 exports.Iterator.prototype.rec = undefined;
/**
* Feature Space
* @classdesc Represents the feature space.
* @class
* @param {module:qm.Base} base - The base where the features are extracted from.
* @param {Array.<Object>} extractors - The extractors.
* @example
* // import qm module
* var qm = require('qminer');
* // construct a base with the store
* var base = new qm.Base({
*	mode: 'createClean',
*	schema: [
*		{ name: 'NewsArticles',
*		  fields: [
*		{ name: "ID", primary: true, type: "string", shortstring: true },
*		{ name: "Source", type: "string", codebook: true }
*		]
*	}]
* });
* // add a record
* base.store('NewsArticles').add({
*	ID: 't12344', 
*	Source: 's1234', 
*	DateTime: '2015-01-01T00:05:00', 
*	Title: 'the title', 
*	Tokens: ['token1', 'token2'], 
*	Vector: [[0,1], [1,1]]});
* // create a feature space 
* var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "NewsArticles", field: "Source" });
*/
 exports.FeatureSpace = function (base, extractors) {};
/**
	* Returns the dimension of the feature space.
	*/
 exports.FeatureSpace.prototype.dim = undefined;
/**
	* Returns an array of the dimensions of each feature extractor in the feature space.
	*/
 exports.FeatureSpace.prototype.dims = undefined;
/**
	* Adds a new feature extractor to the feature space.
	* @param {Object} obj - The added feature extracture.
	* @returns {module:qm.FeatureSpace} Self.
	*/
 exports.FeatureSpace.prototype.add = function (obj) {};
/**
	* Creates a sparse feature vector from the given record.
	* @param {module:qm.Record} rec - The given record.
	* @returns {module:la.SparseVector} The sparse feature vector gained from rec.
	*/
 exports.FeatureSpace.prototype.ftrSpVec = function (rec) {}
/**
	* Creates a feature vector from the given record.
	* @param {module:qm.Record} rec - The given record.
	* @returns {module:la.Vector} The feature vector gained from rec.
	*/
 exports.FeatureSpace.prototype.ftrVec = function (rec) {};
/**
	* Gives the name of feature extractor at given position.
	* @param {number} idx - The index of the feature extractor in feature space (zero based).
	* @returns {String} The name of the feature extractor at position idx.
	*/
 exports.FeatureSpace.prototype.getFtrExtractor = function (idx) {};
/**
	* Gives the name of the feature at the given position.
	* @param {number} idx - The index of the feature in feature space (zero based).
	* @returns {String} THe name of the feature at the position idx.
	*/
 exports.FeatureSpace.prototype.getFtr = function (idx) {};

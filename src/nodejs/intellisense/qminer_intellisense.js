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
    create: 'create',
    createClean: 'createClean',
    open: 'open',
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
	 * @returns {module:la.Store} The store.
	 */
 exports.Base.prototype.store = function (name) { return ''; }
/**
	 * Returns a list of store descriptors.
	 *
	 * @returns {Object[]}
	 */
 exports.Base.prototype.getStoreList = function () { return [{storeId:'', storeName:'', storeRecords:'', fields: [], keys: [], joins: []}]; }
/**
	* Creates a new store.
	* @param {Object} storeDef - The definition of the store(s)
	* @param {number} [storeSizeInMB = 1024] - The reserved size of the store(s).
	* @returns {(module:qm.Store | module:qm.Store[])} - Returns a store or an array of stores (if the schema definition was an array)
	*/
 exports.Base.prototype.createStore = function (storeDef, storeSizeInMB) { return storeDef instanceof Array ? [Object.create(require('qminer').Store.prototype)] : Object.create(require('qminer').Store.prototype) ;}
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
	* // make an array of recod names
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
	* Creates a record set containing random records from store.
	* @param {number} sampleSize - The size of the record set.
	* @returns {Array.<module:qm.Record>} Returns a record set containing a random record set.
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
	* @param {string} fieldName - The field name. Field must be of one-dimensional type, e.g. int, float, string
	* @returns {module:la.Vector} The vector containing the field values of each record.
	*/
 exports.Store.prototype.getVec = function (fieldName) {};
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
* Record
* @classdesc Represents a record object. //TODO new constructor
* @class
*/
 exports.Record = function () {};
/**
* Record Set
* @classdesc Represents the record set object.  TODO new constructor
* @class
* @example
* // import qm module
* var qm = require('qminer');
* // factory based construction using store.recs
* var rs = store.recs;
* // create with constructor
* // TODO
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

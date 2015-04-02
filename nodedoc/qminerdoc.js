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
	* Gives the number of records.
	* @returns {number} Number of records.
	*/
 exports.Store.prototype.length = 0;

/**
* Qminer module.
* @module qm
* @example 
* // import module
* var qm = require('qminer');
*/
/**
	* Creates a directory structure.
	* @param {string} [configPath] - The configuration path. Default: qm.conf
	* @param {boolean} [overwrite] - If you want to overwrite the structure. Default: false
	* @param {number} [portN] - The number of the port. Default: 8080
	* @param {number} [cacheSize] - The space to be reserved? Default: 1024
	* @returns {module:qm.Base}
	*/
 exports.config = function(configPath, overwrite, portN, cacheSize) {}
/**
	* Creates an empty base.
	* @param {string} configPath - Configuration.
	* @param {string} [schemaPath] - Schema.
	* @param {boolean} clear - True, to clear the existing store folder. Otherwise, false.
	* @returns {module:qm.Base}
	*/
 exports.create = function (configPath, schemaPath, clear) {}
/**
	* Opens a base.
	* @param {string} configPath - The configuration.
	* @param {boolean} readOnly - True, if the store should be read only. Otherwise, false.
	* @returns {module:qm.Base}
	*/
 exports.open = function (configPath, readOnly) {}
/**
* Base
* @classdesc Represents the database and holds stores. //TODO constructor
* @class
* @param {string} filePath - The database folder. 
* @param {number} CacheSize - Cache size in MB.
* @example
* // import qm module
* var qm = require('qminer');
* // create a base with the qm configuration file, without the predefined schemas, with overwrite = true
* var base = qm.create('qm.conf', "", true);
* // create a base with constructor
* var base2 = new qm.Base('./db/', 1024);
*/
 exports.Base = function (filePath, CacheSize) {};
/**
	* Closes the database.
	* @returns {null}
	*/
 exports.Base.close = function () {}
/**
	 * Returns the store with the specified name.
	 *
	 * @param {string} name - Name of the store.
	 * @returns {module:la.Store} The store.
	 */
 exports.Base.store = function (name) {}
/**
	 * Returns a list of store descriptors.
	 *
	 * @returns {Object[]}
	 */
/**
	* Creates a new store.
	* @param {Json} storeDef - The definition of the store.
	* @param {number} [storeSizeInMB] - The reserved size of the store.
	* @returns {??}
	*/
 exports.Base.createStore = function (storeDef, storeSizeInMB) {}

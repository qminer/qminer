/**
* Hashtable module.
* @module ht
*/
/**
	* String-int hashmap.
	* @classdesc See {@link module:ht.StrStrMap}. The only difference are the key and data types.
	* @class
	*/
 exports.StrIntMap = function() {}
/**
	* String-float hashmap
	* @classdesc See {@link module:ht.StrStrMap}. The only difference are the key and data types.
	* @class
	*/
 exports.StrFltMap = function() {}
/**
	* Int-string hashmap
	* @classdesc See {@link module:ht.StrStrMap}. The only difference are the key and data types.
	* @class
	*/
 exports.IntStrMap = function() {}
/**
	* Int-int hashmap
	* @classdesc See {@link module:ht.StrStrMap}. The only difference are the key and data types.
	* @class
	*/
 exports.IntIntMap = function() {}
/**
	* Int-float hashmap
	* @classdesc See {@link module:ht.StrStrMap}. The only difference are the key and data types.
	* @class
	*/
 exports.IntFltMap = function() {}
/**
	* String-string hashmap	
	* @classdesc Used for storing key/data pairs, wrapps an efficient C++ implementation.
	* @class
	* @example
	* // create a new string-string hashtable
	* ht = require('qminer').ht;
	* var h = new ht.StrStrMap();
	* // Adding two key/dat pairs
	* h.put('key1', 'val1');
	* h.put('key2', 'val2');
	* // Getting data
	* h.hasKey('key1'); // returns true
	* h.get('key2'); // returns 'val2'
	* h.key(1); // returns 'key2'
	* h.dat(1); // returns 'dat2'
	* h.length; // returns 2
	* // Saving and loading:
	* var fs = require('qminer').fs;
	* fout = fs.openWrite('map.dat'); // open write stream
	* h.save(fout).close(); // save and close write stream
	* var h2 = new ht.StrStrMap(); // new empty table
	* var fin = fs.openRead('map.dat'); // open read stream
	* h2.load(fin); // load
	*/
 exports.StrStrMap = function() {}
/**
	* Returns dat given key
	* @param {string} key - Hashmap key.
	* @returns {string} Hashmap data.
	*/
 exports.StrStrMap.prototype.get = function(key) {}
/**
	* add/update key-value pair
	* @param {string} key - Hashmap key.
	* @param {string} data - Hashmap data.
	* @returns {module:ht.StrStrMap} Self.
	*/
 exports.StrStrMap.prototype.put = function(key, data) {}
/**
	* returns true if the map has a given key 
	* @param {string} key - Hashmap key.	
	* @returns {boolean} True if the map contains key.
	*/
 exports.StrStrMap.prototype.hasKey = function(key) {}
/**
    * @property {number} length - Number of key/dat pairs
	*/
 exports.StrStrMap.prototype.length = undefined;
/**
	* returns n-th key
	* @param {number} n - Hashmap key number.	
	* @returns {string} n-th key.
	*/
 exports.StrStrMap.prototype.key = function(n) {}	
/**
	* returns n-th dat
	* @param {number} n - Hashmap dat number.	
	* @returns {string} n-th data value.
	*/
 exports.StrStrMap.prototype.dat = function(n) {}    
/**
	* loads the hashtable from input stream
	* @param {module:fs.FIn} fin - Input stream.	
	* @returns {module:ht.StrStrMap} Self.
	*/
 exports.StrStrMap.prototype.load = function(fin) {}  	
/**
	* saves the hashtable to output stream
	* @param {module:fs.FOut} fout - Output stream.	
	* @returns {module:fs.FOut} fout.
	*/
 exports.StrStrMap.prototype.save = function(fout) {}  	    


   /**
	* classDesc	
	* @returns {string} desc - Returns class description
	*/
exports.StrStrMap.prototype.classDesc = function () { return 'This class wraps a C++ string-string hash table'; }



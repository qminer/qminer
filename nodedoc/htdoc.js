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
	* @class
	*/
 exports.StrStrMap = function() {}
/**
	* Returns dat given key
	* @param {string} key - Hashmap key.
	* @returns {string} data - Hashmap data.
	*/
 exports.StrStrMap.prototype.get = function(key) {}
/**
	* add/update key-value pair
	* @param {string} key - Hashmap key.
	* @param {string} data - Hashmap data.
	* @returns {module:ht.StrStrMap} this - Returns self.
	*/
 exports.StrStrMap.prototype.put = function(key, data) {}
/**
	* returns true if the map has a given key 
	* @param {string} key - Hashmap key.	
	* @returns {boolean} haskey - Returns true if the map contains key.
	*/
 exports.StrStrMap.prototype.hasKey = function(key) {}
/**
    * @property {number} length Number of key/dat pairs
	*/
 exports.StrStrMap.prototype.length = undefined;
/**
	* returns n-th key
	* @param {number} n - Hashmap key number.	
	* @returns {string} key - Returns n-th key.
	*/
 exports.StrStrMap.prototype.key = function(n) {}	
/**
	* returns n-th dat
	* @param {number} n - Hashmap dat number.	
	* @returns {string} dat - Returns n-th data value.
	*/
 exports.StrStrMap.prototype.dat = function(n) {}    
/**
	* loads the hashtable from input stream
	* @param {module:fs.FIn} fin - Input stream.	
	* @returns {module:ht.StrStrMap} map - Returns map.
	*/
 exports.StrStrMap.prototype.load = function(fin) {}  	
/**
	* saves the hashtable to output stream
	* @param {module:fs.FOut} fout - Output stream.	
	* @returns {module:fs.FOut} fout - Returns the input parameter (so close can be called if needed).
	*/
 exports.StrStrMap.prototype.save = function(fout) {}  	    


   /**
	* classDesc	
	* @returns {string} desc - Returns class description
	*/
exports.StrStrMap.prototype.classDesc = function () { return 'This class wraps a C++ string-string hash table'; }



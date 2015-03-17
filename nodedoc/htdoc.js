/**
* Hashtable module.
* @module ht
*/

/**
	* String-string hashmap 
	* @classdesc Used for storing key/data pairs, wrapps an efficient C++ implementation.
	* @class
	* @example
	* // create a new hashtable
	* ht = require('qminer').ht;
	* var h = new ht.StrStrMap();
	* // Adding two key/dat pairs
	* h.put('foo', 'bar');
	* h.put('dog', 'tisa');
	* // Getting data
	* h.hasKey('foo'); // returns true
	* h.get('dog'); // returns 'tisa'
	* h.key(1); // returns 'dog'
	* h.dat(1); // returns 'tisa'
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
	* @param {number} n - Hashmap key number. Should be between 0 and length-1.	
	* @returns {string} n-th key.
	*/
 exports.StrStrMap.prototype.key = function(n) {}	
/**
	* returns n-th dat
	* @param {number} n - Hashmap dat number. Should be between 0 and length-1
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
	* sorts by keys
	* @param {boolean} [asc=true] - Sort in ascending order?
	*/
 exports.StrStrMap.prototype.sortKey = function(asc) {}
/**
	* sorts by values
	* @param {boolean} [asc=true] - Sort in ascending order?	
	*/
 exports.StrStrMap.prototype.sortDat = function(asc) {}

/**
	* String-Integer hashmap 
	* @classdesc Used for storing key/data pairs, wrapps an efficient C++ implementation.
	* @class
	* @example
	* // create a new hashtable
	* ht = require('qminer').ht;
	* var h = new ht.StrIntMap();
	* // Adding two key/dat pairs
	* h.put('foo', 10);
	* h.put('bar', 20);
	* // Getting data
	* h.hasKey('foo'); // returns true
	* h.get('bar'); // returns 20
	* h.key(1); // returns 'bar'
	* h.dat(1); // returns 20
	* h.length; // returns 2	
	* // Saving and loading:
	* var fs = require('qminer').fs;
	* fout = fs.openWrite('map.dat'); // open write stream
	* h.save(fout).close(); // save and close write stream
	* var h2 = new ht.StrIntMap(); // new empty table
	* var fin = fs.openRead('map.dat'); // open read stream
	* h2.load(fin); // load
	*/
 exports.StrIntMap = function() {}
/**
	* Returns dat given key
	* @param {string} key - Hashmap key.
	* @returns {number} Hashmap data.
	*/
 exports.StrIntMap.prototype.get = function(key) {}
/**
	* add/update key-value pair
	* @param {string} key - Hashmap key.
	* @param {number} data - Hashmap data.
	* @returns {module:ht.StrIntMap} Self.
	*/
 exports.StrIntMap.prototype.put = function(key, data) {}
/**
	* returns true if the map has a given key 
	* @param {string} key - Hashmap key.	
	* @returns {boolean} True if the map contains key.
	*/
 exports.StrIntMap.prototype.hasKey = function(key) {}
/**
    * @property {number} length - Number of key/dat pairs
	*/
 exports.StrIntMap.prototype.length = undefined;
/**
	* returns n-th key
	* @param {number} n - Hashmap key number. Should be between 0 and length-1.	
	* @returns {string} n-th key.
	*/
 exports.StrIntMap.prototype.key = function(n) {}	
/**
	* returns n-th dat
	* @param {number} n - Hashmap dat number. Should be between 0 and length-1
	* @returns {number} n-th data value.
	*/
 exports.StrIntMap.prototype.dat = function(n) {}
/**
	* loads the hashtable from input stream
	* @param {module:fs.FIn} fin - Input stream.	
	* @returns {module:ht.StrIntMap} Self.
	*/
 exports.StrIntMap.prototype.load = function(fin) {}
/**
	* saves the hashtable to output stream
	* @param {module:fs.FOut} fout - Output stream.	
	* @returns {module:fs.FOut} fout.
	*/
 exports.StrIntMap.prototype.save = function(fout) {}
/**
	* sorts by keys
	* @param {boolean} [asc=true] - Sort in ascending order?
	*/
 exports.StrIntMap.prototype.sortKey = function(asc) {}
/**
	* sorts by values
	* @param {boolean} [asc=true] - Sort in ascending order?	
	*/
 exports.StrIntMap.prototype.sortDat = function(asc) {}

/**
	* String-Float hashmap 
	* @classdesc Used for storing key/data pairs, wrapps an efficient C++ implementation.
	* @class
	* @example
	* // create a new hashtable
	* ht = require('qminer').ht;
	* var h = new ht.StrFltMap();
	* // Adding two key/dat pairs
	* h.put('foo', 10.5);
	* h.put('bar', 20.2);
	* // Getting data
	* h.hasKey('foo'); // returns true
	* h.get('bar'); // returns 20.2
	* h.key(1); // returns 'bar'
	* h.dat(1); // returns 20.2
	* h.length; // returns 2	
	* // Saving and loading:
	* var fs = require('qminer').fs;
	* fout = fs.openWrite('map.dat'); // open write stream
	* h.save(fout).close(); // save and close write stream
	* var h2 = new ht.StrFltMap(); // new empty table
	* var fin = fs.openRead('map.dat'); // open read stream
	* h2.load(fin); // load
	*/
 exports.StrFltMap = function() {}
/**
	* Returns dat given key
	* @param {string} key - Hashmap key.
	* @returns {number} Hashmap data.
	*/
 exports.StrFltMap.prototype.get = function(key) {}
/**
	* add/update key-value pair
	* @param {string} key - Hashmap key.
	* @param {number} data - Hashmap data.
	* @returns {module:ht.StrFltMap} Self.
	*/
 exports.StrFltMap.prototype.put = function(key, data) {}
/**
	* returns true if the map has a given key 
	* @param {string} key - Hashmap key.	
	* @returns {boolean} True if the map contains key.
	*/
 exports.StrFltMap.prototype.hasKey = function(key) {}
/**
    * @property {number} length - Number of key/dat pairs
	*/
 exports.StrFltMap.prototype.length = undefined;
/**
	* returns n-th key
	* @param {number} n - Hashmap key number. Should be between 0 and length-1.	
	* @returns {string} n-th key.
	*/
 exports.StrFltMap.prototype.key = function(n) {}	
/**
	* returns n-th dat
	* @param {number} n - Hashmap dat number. Should be between 0 and length-1
	* @returns {number} n-th data value.
	*/
 exports.StrFltMap.prototype.dat = function(n) {}
/**
	* loads the hashtable from input stream
	* @param {module:fs.FIn} fin - Input stream.	
	* @returns {module:ht.StrFltMap} Self.
	*/
 exports.StrFltMap.prototype.load = function(fin) {}
/**
	* saves the hashtable to output stream
	* @param {module:fs.FOut} fout - Output stream.	
	* @returns {module:fs.FOut} fout.
	*/
 exports.StrFltMap.prototype.save = function(fout) {}
/**
	* sorts by keys
	* @param {boolean} [asc=true] - Sort in ascending order?
	*/
 exports.StrFltMap.prototype.sortKey = function(asc) {}
/**
	* sorts by values
	* @param {boolean} [asc=true] - Sort in ascending order?	
	*/
 exports.StrFltMap.prototype.sortDat = function(asc) {}

/**
	* Int-string hashmap 
	* @classdesc Used for storing key/data pairs, wrapps an efficient C++ implementation.
	* @class
	* @example
	* // create a new hashtable
	* ht = require('qminer').ht;
	* var h = new ht.IntStrMap();
	* // Adding two key/dat pairs
	* h.put(10, 'foo');
	* h.put(20, 'bar');
	* // Getting data
	* h.hasKey(10); // returns true
	* h.get(20); // returns 'bar'
	* h.key(1); // returns 20
	* h.dat(1); // returns 'bar'
	* h.length; // returns 2	
	* // Saving and loading:
	* var fs = require('qminer').fs;
	* fout = fs.openWrite('map.dat'); // open write stream
	* h.save(fout).close(); // save and close write stream
	* var h2 = new ht.IntStrMap(); // new empty table
	* var fin = fs.openRead('map.dat'); // open read stream
	* h2.load(fin); // load
	*/
 exports.IntStrMap = function() {}
/**
	* Returns dat given key
	* @param {number} key - Hashmap key.
	* @returns {string} Hashmap data.
	*/
 exports.IntStrMap.prototype.get = function(key) {}
/**
	* add/update key-value pair
	* @param {number} key - Hashmap key.
	* @param {string} data - Hashmap data.
	* @returns {module:ht.IntStrMap} Self.
	*/
 exports.IntStrMap.prototype.put = function(key, data) {}
/**
	* returns true if the map has a given key 
	* @param {number} key - Hashmap key.	
	* @returns {boolean} True if the map contains key.
	*/
 exports.IntStrMap.prototype.hasKey = function(key) {}
/**
    * @property {number} length - Number of key/dat pairs
	*/
 exports.IntStrMap.prototype.length = undefined;
/**
	* returns n-th key
	* @param {number} n - Hashmap key number. Should be between 0 and length-1.	
	* @returns {number} n-th key.
	*/
 exports.IntStrMap.prototype.key = function(n) {}	
/**
	* returns n-th dat
	* @param {number} n - Hashmap dat number. Should be between 0 and length-1
	* @returns {string} n-th data value.
	*/
 exports.IntStrMap.prototype.dat = function(n) {}
/**
	* loads the hashtable from input stream
	* @param {module:fs.FIn} fin - Input stream.	
	* @returns {module:ht.IntStrMap} Self.
	*/
 exports.IntStrMap.prototype.load = function(fin) {}
/**
	* saves the hashtable to output stream
	* @param {module:fs.FOut} fout - Output stream.	
	* @returns {module:fs.FOut} fout.
	*/
 exports.IntStrMap.prototype.save = function(fout) {}
/**
	* sorts by keys
	* @param {boolean} [asc=true] - Sort in ascending order?
	*/
 exports.IntStrMap.prototype.sortKey = function(asc) {}
/**
	* sorts by values
	* @param {boolean} [asc=true] - Sort in ascending order?	
	*/
 exports.IntStrMap.prototype.sortDat = function(asc) {}

/**
	* Integer-Integer hashmap 
	* @classdesc Used for storing key/data pairs, wrapps an efficient C++ implementation.
	* @class
	* @example
	* // create a new hashtable
	* ht = require('qminer').ht;
	* var h = new ht.IntIntMap();
	* // Adding two key/dat pairs
	* h.put(5, 10);
	* h.put(15, 20);
	* // Getting data
	* h.hasKey(5); // returns true
	* h.get(15); // returns 20
	* h.key(1); // returns 15
	* h.dat(1); // returns 20
	* h.length; // returns 2	
	* // Saving and loading:
	* var fs = require('qminer').fs;
	* fout = fs.openWrite('map.dat'); // open write stream
	* h.save(fout).close(); // save and close write stream
	* var h2 = new ht.IntIntMap(); // new empty table
	* var fin = fs.openRead('map.dat'); // open read stream
	* h2.load(fin); // load
	*/
 exports.IntIntMap = function() {}
/**
	* Returns dat given key
	* @param {number} key - Hashmap key.
	* @returns {number} Hashmap data.
	*/
 exports.IntIntMap.prototype.get = function(key) {}
/**
	* add/update key-value pair
	* @param {number} key - Hashmap key.
	* @param {number} data - Hashmap data.
	* @returns {module:ht.IntIntMap} Self.
	*/
 exports.IntIntMap.prototype.put = function(key, data) {}
/**
	* returns true if the map has a given key 
	* @param {number} key - Hashmap key.	
	* @returns {boolean} True if the map contains key.
	*/
 exports.IntIntMap.prototype.hasKey = function(key) {}
/**
    * @property {number} length - Number of key/dat pairs
	*/
 exports.IntIntMap.prototype.length = undefined;
/**
	* returns n-th key
	* @param {number} n - Hashmap key number. Should be between 0 and length-1.	
	* @returns {number} n-th key.
	*/
 exports.IntIntMap.prototype.key = function(n) {}	
/**
	* returns n-th dat
	* @param {number} n - Hashmap dat number. Should be between 0 and length-1
	* @returns {number} n-th data value.
	*/
 exports.IntIntMap.prototype.dat = function(n) {}
/**
	* loads the hashtable from input stream
	* @param {module:fs.FIn} fin - Input stream.	
	* @returns {module:ht.IntIntMap} Self.
	*/
 exports.IntIntMap.prototype.load = function(fin) {}
/**
	* saves the hashtable to output stream
	* @param {module:fs.FOut} fout - Output stream.	
	* @returns {module:fs.FOut} fout.
	*/
 exports.IntIntMap.prototype.save = function(fout) {}
/**
	* sorts by keys
	* @param {boolean} [asc=true] - Sort in ascending order?
	*/
 exports.IntIntMap.prototype.sortKey = function(asc) {}
/**
	* sorts by values
	* @param {boolean} [asc=true] - Sort in ascending order?	
	*/
 exports.IntIntMap.prototype.sortDat = function(asc) {}

/**
	* Integer-Float hashmap 
	* @classdesc Used for storing key/data pairs, wrapps an efficient C++ implementation.
	* @class
	* @example
	* // create a new hashtable
	* ht = require('qminer').ht;
	* var h = new ht.IntFltMap();
	* // Adding two key/dat pairs
	* h.put(5, 10.5);
	* h.put(15, 20.2);
	* // Getting data
	* h.hasKey(5); // returns true
	* h.get(15); // returns 20.2
	* h.key(1); // returns 15
	* h.dat(1); // returns 20.2
	* h.length; // returns 2	
	* // Saving and loading:
	* var fs = require('qminer').fs;
	* fout = fs.openWrite('map.dat'); // open write stream
	* h.save(fout).close(); // save and close write stream
	* var h2 = new ht.IntFltMap(); // new empty table
	* var fin = fs.openRead('map.dat'); // open read stream
	* h2.load(fin); // load
	*/
 exports.IntFltMap = function() {}
/**
	* Returns dat given key
	* @param {number} key - Hashmap key.
	* @returns {number} Hashmap data.
	*/
 exports.IntFltMap.prototype.get = function(key) {}
/**
	* add/update key-value pair
	* @param {number} key - Hashmap key.
	* @param {number} data - Hashmap data.
	* @returns {module:ht.IntFltMap} Self.
	*/
 exports.IntFltMap.prototype.put = function(key, data) {}
/**
	* returns true if the map has a given key 
	* @param {number} key - Hashmap key.	
	* @returns {boolean} True if the map contains key.
	*/
 exports.IntFltMap.prototype.hasKey = function(key) {}
/**
    * @property {number} length - Number of key/dat pairs
	*/
 exports.IntFltMap.prototype.length = undefined;
/**
	* returns n-th key
	* @param {number} n - Hashmap key number. Should be between 0 and length-1.	
	* @returns {number} n-th key.
	*/
 exports.IntFltMap.prototype.key = function(n) {}	
/**
	* returns n-th dat
	* @param {number} n - Hashmap dat number. Should be between 0 and length-1
	* @returns {number} n-th data value.
	*/
 exports.IntFltMap.prototype.dat = function(n) {}
/**
	* loads the hashtable from input stream
	* @param {module:fs.FIn} fin - Input stream.	
	* @returns {module:ht.IntFltMap} Self.
	*/
 exports.IntFltMap.prototype.load = function(fin) {}
/**
	* saves the hashtable to output stream
	* @param {module:fs.FOut} fout - Output stream.	
	* @returns {module:fs.FOut} fout.
	*/
 exports.IntFltMap.prototype.save = function(fout) {}
/**
	* sorts by keys
	* @param {boolean} [asc=true] - Sort in ascending order?
	*/
 exports.IntFltMap.prototype.sortKey = function(asc) {}
/**
	* sorts by values
	* @param {boolean} [asc=true] - Sort in ascending order?	
	*/
 exports.IntFltMap.prototype.sortDat = function(asc) {}



   /**
	* classDesc	
	* @returns {string} desc - Returns class description
	*/
exports.StrStrMap.prototype.classDesc = function () { return 'This class wraps a C++ string-string hash table'; }



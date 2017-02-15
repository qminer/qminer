/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

/**
* Hashtable module.
* @module ht
*/

/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
/**
	* String-string hashmap. 
	* @classdesc Used for storing key/data pairs, wraps an efficient C++ implementation.
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
	* Returns dat given key.
	* @param {string} key - Hashmap key.
	* @returns {string} Hashmap data.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.StrStrMap();
    * // add a key/dat pair
    * h.put('foo', 'bar');
    * // get the newly added data
    * var val = h.get('foo'); // returns 'bar'
	*/
 exports.StrStrMap.prototype.get = function(key) { return ; }
/**
	* Add/update key-value pair.
	* @param {string} key - Hashmap key.
	* @param {string} data - Hashmap data.
	* @returns {module:ht.StrStrMap} Self.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.StrStrMap();
    * // add a key/dat pair
    * h.put('foo', 'bar');
	*/
 exports.StrStrMap.prototype.put = function(key, data) { return this; }
/**
	* Returns true if the map has a given key.
	* @param {string} key - Hashmap key.	
	* @returns {boolean} True if the map contains key. Otherwise, false.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.StrStrMap();
    * // add a key/dat pair
    * h.put('foo', 'bar');
    * // check if the hashtable has the key
    * h.hasKey('foo'); // returns true
	*/
 exports.StrStrMap.prototype.hasKey = function(key) { return false; }
/**
    * Number of key/dat pairs. Type `number`.
    * @example
    * // create a new hashtable
	* ht = require('qminer').ht;
	* var h = new ht.StrStrMap();
	* // Adding two key/dat pairs
	* h.put('foo', 'bar');
    * // get the number of key/dat pairs
    * var length = h.length; // returns 1
	*/
 exports.StrStrMap.prototype.length = 0;
/**
	* Returns n-th key.
	* @param {number} n - Hashmap key index number. Should be between 0 and length-1.	
	* @returns {string} The n-th key.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.StrStrMap();
    * // add a key/dat pair
    * h.put('foo', 'bar');
    * // get the first key
    * var key = h.key(0); // returns 'foo'
	*/
 exports.StrStrMap.prototype.key = function(n) { return ; }	
/**
    * Returns the ID of the key provided as parameter.
    * @param {string} key - Hashmap key.
    * @returns {number} n - Hashmap index number of the key.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.StrStrMap();
    * // add a key/dat pair
    * h.put('foo', 'bar');
    * // get key id of 'foo' 
    * var key = h.keyid('foo'); // returns 0
    */
 exports.StrStrMap.prototype.key = function(n) { return ; }	
/**
	* Returns n-th dat.
	* @param {number} n - Hashmap dat index number. Should be between 0 and length-1.
	* @returns {string} The n-th data value.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.StrStrMap();
    * // add a key/dat pair
    * h.put('foo', 'bar');
    * // get the first dat
    * var key = h.key(0); // returns 'bar'
	*/
 exports.StrStrMap.prototype.dat = function(n) { return ; }
/**
	* Loads the hashtable from input stream.
	* @param {module:fs.FIn} fin - Input stream.	
	* @returns {module:ht.StrStrMap} Self.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * var fs = qm.fs;
    * // create a new hashtable
    * var h = new ht.StrStrMap();
    * fout = fs.openWrite('map.dat'); // open write stream
    * h.save(fout).close(); // save and close write stream
    * var fin = fs.openRead('map.dat'); // open read stream
    * var h2 = new ht.StrStrMap();
    * h2.load(fin); // load
	*/
 exports.StrStrMap.prototype.load = function(fin) { return this; }
/**
	* Saves the hashtable to output stream.
	* @param {module:fs.FOut} fout - Output stream.	
	* @returns {module:fs.FOut} fout.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * var fs = qm.fs;
    * // create a new hashtable
    * var h = new ht.StrStrMap();
    * fout = fs.openWrite('map.dat'); // open write stream
    * h.save(fout).close(); // save and close write stream
	*/
 exports.StrStrMap.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); }
/**
	* Sorts by keys.
	* @param {boolean} [asc=true] - If true, sorts in ascending order.
	* @returns {module:ht.StrStrMap} Self.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.StrStrMap();
    * h.put('foo', 'bar');
    * h.put('dog', 'tisa');
    * // sort the hashtable by keys
    * h.sortKey();
	*/
 exports.StrStrMap.prototype.sortKey = function(asc) { return this; }
/**
	* Sorts by dat.
	* @param {boolean} [asc=true] - If true, sorts in ascending order.
	* @returns {module:ht.StrStrMap} Self.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.StrStrMap();
    * h.put('foo', 'bar');
    * h.put('dog', 'tisa');
    * // sort the hashtable by dat
    * h.sortDat();
	*/
 exports.StrStrMap.prototype.sortDat = function(asc) { return this; }

/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
/**
	* String-Integer hashmap. 
	* @classdesc Used for storing key/data pairs, wraps an efficient C++ implementation.
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
	* Returns dat given key.
	* @param {string} key - Hashmap key.
	* @returns {number} Hashmap data.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.StrIntMap();
    * // add a key/dat pair
    * h.put('foo', 10);
    * // get the newly added data
    * var val = h.get('foo'); // returns 10
	*/
 exports.StrIntMap.prototype.get = function(key) { return 0; }
/**
	* Add/update key-value pair.
	* @param {string} key - Hashmap key.
	* @param {number} data - Hashmap data.
	* @returns {module:ht.StrIntMap} Self.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.StrIntMap();
    * // add a key/dat pair
    * h.put('foo', 10);
	*/
 exports.StrIntMap.prototype.put = function(key, data) { return this; }
/**
	* Returns true if the map has a given key.
	* @param {string} key - Hashmap key.	
	* @returns {boolean} True if the map contains key. Otherwise, false.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.StrIntMap();
    * // add a key/dat pair
    * h.put('foo', 10);
    * // check if the hashtable has the key
    * h.hasKey('foo'); // returns true
	*/
 exports.StrIntMap.prototype.hasKey = function(key) { return false; }
/**
    * Number of key/dat pairs. Type `number`.
    * @example
    * // create a new hashtable
	* ht = require('qminer').ht;
	* var h = new ht.StrIntMap();
	* // Adding two key/dat pairs
	* h.put('foo', 10);
    * // get the number of key/dat pairs
    * var length = h.length; // returns 1
	*/
 exports.StrIntMap.prototype.length = 0;
/**
	* Returns n-th key.
	* @param {number} n - Hashmap key index number. Should be between 0 and length-1.	
	* @returns {string} The n-th key.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.StrIntMap();
    * // add a key/dat pair
    * h.put('foo', 10);
    * // get the first key
    * var key = h.key(0); // returns 'foo'
	*/
 exports.StrIntMap.prototype.key = function(n) { return ; }	
/**
    * Returns the ID of the key provided as parameter.
    * @param {string} key - Hashmap key.
    * @returns {number} n - Hashmap index number of the key.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.StrIntMap();
    * // add a key/dat pair
    * h.put('foo', 10);
    * // get key id of 'foo' 
    * var key = h.keyid('foo'); // returns 0
    */
 exports.StrIntMap.prototype.key = function(n) { return ; }	
/**
	* Returns n-th dat.
	* @param {number} n - Hashmap dat index number. Should be between 0 and length-1.
	* @returns {number} The n-th data value.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.StrIntMap();
    * // add a key/dat pair
    * h.put('foo', 10);
    * // get the first dat
    * var key = h.key(0); // returns 10
	*/
 exports.StrIntMap.prototype.dat = function(n) { return 0; }
/**
	* Loads the hashtable from input stream.
	* @param {module:fs.FIn} fin - Input stream.	
	* @returns {module:ht.StrIntMap} Self.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * var fs = qm.fs;
    * // create a new hashtable
    * var h = new ht.StrIntMap();
    * fout = fs.openWrite('map.dat'); // open write stream
    * h.save(fout).close(); // save and close write stream
    * var fin = fs.openRead('map.dat'); // open read stream
    * var h2 = new ht.StrIntMap();
    * h2.load(fin); // load
	*/
 exports.StrIntMap.prototype.load = function(fin) { return this; }
/**
	* Saves the hashtable to output stream.
	* @param {module:fs.FOut} fout - Output stream.	
	* @returns {module:fs.FOut} fout.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * var fs = qm.fs;
    * // create a new hashtable
    * var h = new ht.StrIntMap();
    * fout = fs.openWrite('map.dat'); // open write stream
    * h.save(fout).close(); // save and close write stream
	*/
 exports.StrIntMap.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); }
/**
	* Sorts by keys.
	* @param {boolean} [asc=true] - If true, sorts in ascending order.
	* @returns {module:ht.StrIntMap} Self.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.StrIntMap();
    * h.put('foo', 10);
    * h.put('bar', 20);
    * // sort the hashtable by keys
    * h.sortKey();
	*/
 exports.StrIntMap.prototype.sortKey = function(asc) { return this; }
/**
	* Sorts by dat.
	* @param {boolean} [asc=true] - If true, sorts in ascending order.
	* @returns {module:ht.StrIntMap} Self.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.StrIntMap();
    * h.put('foo', 10);
    * h.put('bar', 20);
    * // sort the hashtable by dat
    * h.sortDat();
	*/
 exports.StrIntMap.prototype.sortDat = function(asc) { return this; }

/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
/**
	* String-Float hashmap. 
	* @classdesc Used for storing key/data pairs, wraps an efficient C++ implementation.
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
	* Returns dat given key.
	* @param {string} key - Hashmap key.
	* @returns {number} Hashmap data.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.StrFltMap();
    * // add a key/dat pair
    * h.put('foo', 10.5);
    * // get the newly added data
    * var val = h.get('foo'); // returns 10.5
	*/
 exports.StrFltMap.prototype.get = function(key) { return 0; }
/**
	* Add/update key-value pair.
	* @param {string} key - Hashmap key.
	* @param {number} data - Hashmap data.
	* @returns {module:ht.StrFltMap} Self.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.StrFltMap();
    * // add a key/dat pair
    * h.put('foo', 10.5);
	*/
 exports.StrFltMap.prototype.put = function(key, data) { return this; }
/**
	* Returns true if the map has a given key.
	* @param {string} key - Hashmap key.	
	* @returns {boolean} True if the map contains key. Otherwise, false.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.StrFltMap();
    * // add a key/dat pair
    * h.put('foo', 10.5);
    * // check if the hashtable has the key
    * h.hasKey('foo'); // returns true
	*/
 exports.StrFltMap.prototype.hasKey = function(key) { return false; }
/**
    * Number of key/dat pairs. Type `number`.
    * @example
    * // create a new hashtable
	* ht = require('qminer').ht;
	* var h = new ht.StrFltMap();
	* // Adding two key/dat pairs
	* h.put('foo', 10.5);
    * // get the number of key/dat pairs
    * var length = h.length; // returns 1
	*/
 exports.StrFltMap.prototype.length = 0;
/**
	* Returns n-th key.
	* @param {number} n - Hashmap key index number. Should be between 0 and length-1.	
	* @returns {string} The n-th key.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.StrFltMap();
    * // add a key/dat pair
    * h.put('foo', 10.5);
    * // get the first key
    * var key = h.key(0); // returns 'foo'
	*/
 exports.StrFltMap.prototype.key = function(n) { return ; }	
/**
    * Returns the ID of the key provided as parameter.
    * @param {string} key - Hashmap key.
    * @returns {number} n - Hashmap index number of the key.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.StrFltMap();
    * // add a key/dat pair
    * h.put('foo', 10.5);
    * // get key id of 'foo' 
    * var key = h.keyid('foo'); // returns 0
    */
 exports.StrFltMap.prototype.key = function(n) { return ; }	
/**
	* Returns n-th dat.
	* @param {number} n - Hashmap dat index number. Should be between 0 and length-1.
	* @returns {number} The n-th data value.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.StrFltMap();
    * // add a key/dat pair
    * h.put('foo', 10.5);
    * // get the first dat
    * var key = h.key(0); // returns 10.5
	*/
 exports.StrFltMap.prototype.dat = function(n) { return 0; }
/**
	* Loads the hashtable from input stream.
	* @param {module:fs.FIn} fin - Input stream.	
	* @returns {module:ht.StrFltMap} Self.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * var fs = qm.fs;
    * // create a new hashtable
    * var h = new ht.StrFltMap();
    * fout = fs.openWrite('map.dat'); // open write stream
    * h.save(fout).close(); // save and close write stream
    * var fin = fs.openRead('map.dat'); // open read stream
    * var h2 = new ht.StrFltMap();
    * h2.load(fin); // load
	*/
 exports.StrFltMap.prototype.load = function(fin) { return this; }
/**
	* Saves the hashtable to output stream.
	* @param {module:fs.FOut} fout - Output stream.	
	* @returns {module:fs.FOut} fout.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * var fs = qm.fs;
    * // create a new hashtable
    * var h = new ht.StrFltMap();
    * fout = fs.openWrite('map.dat'); // open write stream
    * h.save(fout).close(); // save and close write stream
	*/
 exports.StrFltMap.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); }
/**
	* Sorts by keys.
	* @param {boolean} [asc=true] - If true, sorts in ascending order.
	* @returns {module:ht.StrFltMap} Self.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.StrFltMap();
    * h.put('foo', 10.5);
    * h.put('bar', 20.2);
    * // sort the hashtable by keys
    * h.sortKey();
	*/
 exports.StrFltMap.prototype.sortKey = function(asc) { return this; }
/**
	* Sorts by dat.
	* @param {boolean} [asc=true] - If true, sorts in ascending order.
	* @returns {module:ht.StrFltMap} Self.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.StrFltMap();
    * h.put('foo', 10.5);
    * h.put('bar', 20.2);
    * // sort the hashtable by dat
    * h.sortDat();
	*/
 exports.StrFltMap.prototype.sortDat = function(asc) { return this; }

/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
/**
	* Int-string hashmap. 
	* @classdesc Used for storing key/data pairs, wraps an efficient C++ implementation.
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
	* Returns dat given key.
	* @param {number} key - Hashmap key.
	* @returns {string} Hashmap data.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.IntStrMap();
    * // add a key/dat pair
    * h.put(10, 'foo');
    * // get the newly added data
    * var val = h.get(10); // returns 'foo'
	*/
 exports.IntStrMap.prototype.get = function(key) { return ; }
/**
	* Add/update key-value pair.
	* @param {number} key - Hashmap key.
	* @param {string} data - Hashmap data.
	* @returns {module:ht.IntStrMap} Self.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.IntStrMap();
    * // add a key/dat pair
    * h.put(10, 'foo');
	*/
 exports.IntStrMap.prototype.put = function(key, data) { return this; }
/**
	* Returns true if the map has a given key.
	* @param {number} key - Hashmap key.	
	* @returns {boolean} True if the map contains key. Otherwise, false.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.IntStrMap();
    * // add a key/dat pair
    * h.put(10, 'foo');
    * // check if the hashtable has the key
    * h.hasKey(10); // returns true
	*/
 exports.IntStrMap.prototype.hasKey = function(key) { return false; }
/**
    * Number of key/dat pairs. Type `number`.
    * @example
    * // create a new hashtable
	* ht = require('qminer').ht;
	* var h = new ht.IntStrMap();
	* // Adding two key/dat pairs
	* h.put(10, 'foo');
    * // get the number of key/dat pairs
    * var length = h.length; // returns 1
	*/
 exports.IntStrMap.prototype.length = 0;
/**
	* Returns n-th key.
	* @param {number} n - Hashmap key index number. Should be between 0 and length-1.	
	* @returns {number} The n-th key.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.IntStrMap();
    * // add a key/dat pair
    * h.put(10, 'foo');
    * // get the first key
    * var key = h.key(0); // returns 10
	*/
 exports.IntStrMap.prototype.key = function(n) { return 0; }	
/**
    * Returns the ID of the key provided as parameter.
    * @param {number} key - Hashmap key.
    * @returns {number} n - Hashmap index number of the key.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.IntStrMap();
    * // add a key/dat pair
    * h.put(10, 'foo');
    * // get key id of 10 
    * var key = h.keyid(10); // returns 0
    */
 exports.IntStrMap.prototype.key = function(n) { return 0; }	
/**
	* Returns n-th dat.
	* @param {number} n - Hashmap dat index number. Should be between 0 and length-1.
	* @returns {string} The n-th data value.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.IntStrMap();
    * // add a key/dat pair
    * h.put(10, 'foo');
    * // get the first dat
    * var key = h.key(0); // returns 'foo'
	*/
 exports.IntStrMap.prototype.dat = function(n) { return ; }
/**
	* Loads the hashtable from input stream.
	* @param {module:fs.FIn} fin - Input stream.	
	* @returns {module:ht.IntStrMap} Self.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * var fs = qm.fs;
    * // create a new hashtable
    * var h = new ht.IntStrMap();
    * fout = fs.openWrite('map.dat'); // open write stream
    * h.save(fout).close(); // save and close write stream
    * var fin = fs.openRead('map.dat'); // open read stream
    * var h2 = new ht.IntStrMap();
    * h2.load(fin); // load
	*/
 exports.IntStrMap.prototype.load = function(fin) { return this; }
/**
	* Saves the hashtable to output stream.
	* @param {module:fs.FOut} fout - Output stream.	
	* @returns {module:fs.FOut} fout.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * var fs = qm.fs;
    * // create a new hashtable
    * var h = new ht.IntStrMap();
    * fout = fs.openWrite('map.dat'); // open write stream
    * h.save(fout).close(); // save and close write stream
	*/
 exports.IntStrMap.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); }
/**
	* Sorts by keys.
	* @param {boolean} [asc=true] - If true, sorts in ascending order.
	* @returns {module:ht.IntStrMap} Self.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.IntStrMap();
    * h.put(10, 'foo');
    * h.put(20, 'bar');
    * // sort the hashtable by keys
    * h.sortKey();
	*/
 exports.IntStrMap.prototype.sortKey = function(asc) { return this; }
/**
	* Sorts by dat.
	* @param {boolean} [asc=true] - If true, sorts in ascending order.
	* @returns {module:ht.IntStrMap} Self.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.IntStrMap();
    * h.put(10, 'foo');
    * h.put(20, 'bar');
    * // sort the hashtable by dat
    * h.sortDat();
	*/
 exports.IntStrMap.prototype.sortDat = function(asc) { return this; }

/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
/**
	* Integer-Integer hashmap. 
	* @classdesc Used for storing key/data pairs, wraps an efficient C++ implementation.
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
	* Returns dat given key.
	* @param {number} key - Hashmap key.
	* @returns {number} Hashmap data.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.IntIntMap();
    * // add a key/dat pair
    * h.put(5, 10);
    * // get the newly added data
    * var val = h.get(5); // returns 10
	*/
 exports.IntIntMap.prototype.get = function(key) { return 0; }
/**
	* Add/update key-value pair.
	* @param {number} key - Hashmap key.
	* @param {number} data - Hashmap data.
	* @returns {module:ht.IntIntMap} Self.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.IntIntMap();
    * // add a key/dat pair
    * h.put(5, 10);
	*/
 exports.IntIntMap.prototype.put = function(key, data) { return this; }
/**
	* Returns true if the map has a given key.
	* @param {number} key - Hashmap key.	
	* @returns {boolean} True if the map contains key. Otherwise, false.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.IntIntMap();
    * // add a key/dat pair
    * h.put(5, 10);
    * // check if the hashtable has the key
    * h.hasKey(5); // returns true
	*/
 exports.IntIntMap.prototype.hasKey = function(key) { return false; }
/**
    * Number of key/dat pairs. Type `number`.
    * @example
    * // create a new hashtable
	* ht = require('qminer').ht;
	* var h = new ht.IntIntMap();
	* // Adding two key/dat pairs
	* h.put(5, 10);
    * // get the number of key/dat pairs
    * var length = h.length; // returns 1
	*/
 exports.IntIntMap.prototype.length = 0;
/**
	* Returns n-th key.
	* @param {number} n - Hashmap key index number. Should be between 0 and length-1.	
	* @returns {number} The n-th key.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.IntIntMap();
    * // add a key/dat pair
    * h.put(5, 10);
    * // get the first key
    * var key = h.key(0); // returns 5
	*/
 exports.IntIntMap.prototype.key = function(n) { return 0; }	
/**
    * Returns the ID of the key provided as parameter.
    * @param {number} key - Hashmap key.
    * @returns {number} n - Hashmap index number of the key.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.IntIntMap();
    * // add a key/dat pair
    * h.put(5, 10);
    * // get key id of 5 
    * var key = h.keyid(5); // returns 0
    */
 exports.IntIntMap.prototype.key = function(n) { return 0; }	
/**
	* Returns n-th dat.
	* @param {number} n - Hashmap dat index number. Should be between 0 and length-1.
	* @returns {number} The n-th data value.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.IntIntMap();
    * // add a key/dat pair
    * h.put(5, 10);
    * // get the first dat
    * var key = h.key(0); // returns 10
	*/
 exports.IntIntMap.prototype.dat = function(n) { return 0; }
/**
	* Loads the hashtable from input stream.
	* @param {module:fs.FIn} fin - Input stream.	
	* @returns {module:ht.IntIntMap} Self.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * var fs = qm.fs;
    * // create a new hashtable
    * var h = new ht.IntIntMap();
    * fout = fs.openWrite('map.dat'); // open write stream
    * h.save(fout).close(); // save and close write stream
    * var fin = fs.openRead('map.dat'); // open read stream
    * var h2 = new ht.IntIntMap();
    * h2.load(fin); // load
	*/
 exports.IntIntMap.prototype.load = function(fin) { return this; }
/**
	* Saves the hashtable to output stream.
	* @param {module:fs.FOut} fout - Output stream.	
	* @returns {module:fs.FOut} fout.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * var fs = qm.fs;
    * // create a new hashtable
    * var h = new ht.IntIntMap();
    * fout = fs.openWrite('map.dat'); // open write stream
    * h.save(fout).close(); // save and close write stream
	*/
 exports.IntIntMap.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); }
/**
	* Sorts by keys.
	* @param {boolean} [asc=true] - If true, sorts in ascending order.
	* @returns {module:ht.IntIntMap} Self.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.IntIntMap();
    * h.put(5, 10);
    * h.put(15, 20);
    * // sort the hashtable by keys
    * h.sortKey();
	*/
 exports.IntIntMap.prototype.sortKey = function(asc) { return this; }
/**
	* Sorts by dat.
	* @param {boolean} [asc=true] - If true, sorts in ascending order.
	* @returns {module:ht.IntIntMap} Self.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.IntIntMap();
    * h.put(5, 10);
    * h.put(15, 20);
    * // sort the hashtable by dat
    * h.sortDat();
	*/
 exports.IntIntMap.prototype.sortDat = function(asc) { return this; }

/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
/**
	* Integer-Float hashmap. 
	* @classdesc Used for storing key/data pairs, wraps an efficient C++ implementation.
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
	* Returns dat given key.
	* @param {number} key - Hashmap key.
	* @returns {number} Hashmap data.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.IntFltMap();
    * // add a key/dat pair
    * h.put(5, 10.5);
    * // get the newly added data
    * var val = h.get(5); // returns 10.5
	*/
 exports.IntFltMap.prototype.get = function(key) { return 0; }
/**
	* Add/update key-value pair.
	* @param {number} key - Hashmap key.
	* @param {number} data - Hashmap data.
	* @returns {module:ht.IntFltMap} Self.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.IntFltMap();
    * // add a key/dat pair
    * h.put(5, 10.5);
	*/
 exports.IntFltMap.prototype.put = function(key, data) { return this; }
/**
	* Returns true if the map has a given key.
	* @param {number} key - Hashmap key.	
	* @returns {boolean} True if the map contains key. Otherwise, false.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.IntFltMap();
    * // add a key/dat pair
    * h.put(5, 10.5);
    * // check if the hashtable has the key
    * h.hasKey(5); // returns true
	*/
 exports.IntFltMap.prototype.hasKey = function(key) { return false; }
/**
    * Number of key/dat pairs. Type `number`.
    * @example
    * // create a new hashtable
	* ht = require('qminer').ht;
	* var h = new ht.IntFltMap();
	* // Adding two key/dat pairs
	* h.put(5, 10.5);
    * // get the number of key/dat pairs
    * var length = h.length; // returns 1
	*/
 exports.IntFltMap.prototype.length = 0;
/**
	* Returns n-th key.
	* @param {number} n - Hashmap key index number. Should be between 0 and length-1.	
	* @returns {number} The n-th key.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.IntFltMap();
    * // add a key/dat pair
    * h.put(5, 10.5);
    * // get the first key
    * var key = h.key(0); // returns 5
	*/
 exports.IntFltMap.prototype.key = function(n) { return 0; }	
/**
    * Returns the ID of the key provided as parameter.
    * @param {number} key - Hashmap key.
    * @returns {number} n - Hashmap index number of the key.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.IntFltMap();
    * // add a key/dat pair
    * h.put(5, 10.5);
    * // get key id of 5 
    * var key = h.keyid(5); // returns 0
    */
 exports.IntFltMap.prototype.key = function(n) { return 0; }	
/**
	* Returns n-th dat.
	* @param {number} n - Hashmap dat index number. Should be between 0 and length-1.
	* @returns {number} The n-th data value.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.IntFltMap();
    * // add a key/dat pair
    * h.put(5, 10.5);
    * // get the first dat
    * var key = h.key(0); // returns 10.5
	*/
 exports.IntFltMap.prototype.dat = function(n) { return 0; }
/**
	* Loads the hashtable from input stream.
	* @param {module:fs.FIn} fin - Input stream.	
	* @returns {module:ht.IntFltMap} Self.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * var fs = qm.fs;
    * // create a new hashtable
    * var h = new ht.IntFltMap();
    * fout = fs.openWrite('map.dat'); // open write stream
    * h.save(fout).close(); // save and close write stream
    * var fin = fs.openRead('map.dat'); // open read stream
    * var h2 = new ht.IntFltMap();
    * h2.load(fin); // load
	*/
 exports.IntFltMap.prototype.load = function(fin) { return this; }
/**
	* Saves the hashtable to output stream.
	* @param {module:fs.FOut} fout - Output stream.	
	* @returns {module:fs.FOut} fout.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * var fs = qm.fs;
    * // create a new hashtable
    * var h = new ht.IntFltMap();
    * fout = fs.openWrite('map.dat'); // open write stream
    * h.save(fout).close(); // save and close write stream
	*/
 exports.IntFltMap.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); }
/**
	* Sorts by keys.
	* @param {boolean} [asc=true] - If true, sorts in ascending order.
	* @returns {module:ht.IntFltMap} Self.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.IntFltMap();
    * h.put(5, 10.5);
    * h.put(15, 20.2);
    * // sort the hashtable by keys
    * h.sortKey();
	*/
 exports.IntFltMap.prototype.sortKey = function(asc) { return this; }
/**
	* Sorts by dat.
	* @param {boolean} [asc=true] - If true, sorts in ascending order.
	* @returns {module:ht.IntFltMap} Self.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.IntFltMap();
    * h.put(5, 10.5);
    * h.put(15, 20.2);
    * // sort the hashtable by dat
    * h.sortDat();
	*/
 exports.IntFltMap.prototype.sortDat = function(asc) { return this; }



   /**
	* classDesc	
	* @returns {string} desc - Returns class description
	*/
exports.StrStrMap.prototype.classDesc = function () { return 'This class wraps a C++ string-string hash table'; }



// QMiner - Open Source Analytics Platform
// 
// Copyright (C) 2014 Jozef Stefan Institute
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License, version 3,
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

//////////////////////////////////////////
// QMiner 

// loading data into stores
qm.load = function() {
	var _obj = {};
	
    //# - `qm.load.jsonFileLimit(store, fileName, limit)` -- load file `fileName` 
    //#   line by line, parsing each line as JSON and adding it as record to `store`.
    //#   When `limit != -1` only first first `limit` lines are loaded
	_obj.jsonFileLimit = function (store, file, limit) {
		var fin = fs.openRead(file);
		var count = 0;
		while (!fin.eof) {
			var line = fin.getNextLn();
			if (line == "") { continue; }
			try {
				var rec = JSON.parse(line);                
				store.add(rec);
				// count, GC and report
				count++;
				if (count % 1000 == 0) { 
					qm.gc();
				}
				if (count % 10000 == 0) {
					console.log("  " + count + " records"); 
				}
                if (count == limit) {
                    break;
                }
			} catch (err) {
				console.log("Error parsing [" + line + "]: " + err)
			}
		}
        console.log("Loaded " + count + " records to " + store.name);
		return count;
	}

    //# - `qm.load.jsonFile(store, fileName)` -- load file `fileName` line by line, 
    //#   parsing each line as JSON and adding it as record to `store`
	_obj.jsonFile = function (store, file) {
        return _obj.jsonFileLimit(store, file, -1);
    }
    
    return _obj;
}();

//# - `qm.printStreamAggr(store)` -- prints all current field values of every stream aggregate attached to the store `store`
qm.printStreamAggr = function(store) {
	var names = store.getStreamAggrNames();
	console.print("[store name] : [streamAggr name] : [field name] : [typeof value] : [value]\n");
	for (var saggrN = 0; saggrN < names.length; saggrN++) {
		var saggr = store.getStreamAggr(names[saggrN]);
		var keys = Object.keys(saggr);
		for (var keyN = 0; keyN < keys.length; keyN++) {
			console.print(store.name + " : " + names[saggrN] + " : " + keys[keyN] + " : " +  typeof(saggr[keys[keyN]]) +  " : " + saggr[keys[keyN]] + "\n");
		}
	}
}

//# - `dir(obj, depth, perfix, limit)` -- recursively prints all keys of object `obj`. Depth of recursion is controlled by `depth` (integer, default 1), `prefix` is a string attached to every line (default empty string), width is controlled by `limit` (integer, default 10)
function dir(obj, depth, prefix, limit) {
	depth = typeof depth !== 'undefined' ? depth : 1;
	prefix = typeof prefix !== 'undefined' ? prefix : "";
	limit = typeof limit !== 'undefined' ? limit : 10;
	if (depth === parseInt(depth)) {
		if (depth > 0) {
			if (typeof obj == 'object') {
				var keys = Object.keys(obj);			
				for (var keyN = 0; keyN < Math.min(limit, keys.length); keyN++) {
					console.println(prefix + "." + keys[keyN]);
					dir(obj[keys[keyN]], depth - 1, prefix + "." + keys[keyN]);
				}	    	
			}
		}
	}	 
}

///////////////////////////////////////// DEPRECATED
function jsonp(req, res, data) {
    console.log("Warning: jsonp is deprecated, please use http.jsonp instead");
    http.jsonp(req, res, data);
}

////////////////////////////////////////// DEPRECATED
// Hash table
function hashTable() {
    console.log("Warning: hashTable is deprecated 28.5.2014, please use require('utilities.js').hashTable instead");
    this._data = new Object();
    this.keys = new Array();
    this.vals = new Array();
    this.put = function (key) { this._data[key] = ""; this.keys.push(key); }
    this.put = function (key, dat) { this._data[key] = dat; this.keys.push(key); this.vals.push(dat); }
    this.contains = function (key) { return this._data.hasOwnProperty(key); }
    this.get = function (key) { return this._data.hasOwnProperty(key) ? this._data[key] : null; }
}

////////////////////////////////////////// 
// deprecated, this are temporary placeholders for warnings so old stuff doesn't break
function isObject(arg) {
    console.log("Warning: isObject is deprecated, please use require('utilities.js').isObject instead");
    return require("utilities.js").isObject(arg);
}

function isArray(arg) {
    console.log("Warning: isObject is deprecated, please use require('utilities.js').isArray instead");
    return require("utilities.js").isArray(arg);
}

function isNumber(n) {
    console.log("Warning: isObject is deprecated, please use require('utilities.js').isNumber instead");
    return require("utilities.js").isNumber(n);
}

function isString(s) {
    console.log("Warning: isObject is deprecated, please use require('utilities.js').isString instead");
    return require("utilities.js").isString(s);
}

function ifNull(val, defVal) {
    console.log("Warning: isObject is deprecated, please use require('utilities.js').ifNull instead");
    return require("utilities.js").ifNull(val, defVal);
}

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
	
    //#- `num = qm.load.jsonFileLimit(store, fileName, limit)` -- load file `fileName` 
    //#   line by line, parsing each line as JSON and adding it as record to `store`.
    //#   When `limit != -1` only first first `limit` lines are loaded. Returns `num`:
    //#   the number of lines loaded.
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

    //#- `num = qm.load.jsonFile(store, fileName)` -- load file `fileName` line by line, 
    //#   parsing each line as JSON and adding it as record to `store`. Returns `num`:
    //#   the number of lines loaded.
	_obj.jsonFile = function (store, file) {
        return _obj.jsonFileLimit(store, file, -1);
    }
    
    return _obj;
}();

//#- `qm.printStreamAggr(store)` -- prints all current field values of every stream aggregate attached to the store `store`
qm.printStreamAggr = function (store) {
    var onStore = arguments.length > 0
    if (!onStore) {
        var names = qm.getStreamAggrNames();
    } else {
        var names = store.getStreamAggrNames();
    }    
	console.print("[store name] : [streamAggr name] : [field name] : [typeof value] : [value]\n");
	for (var saggrN = 0; saggrN < names.length; saggrN++) {
		var saggr = onStore ? store.getStreamAggr(names[saggrN]).val : qm.getStreamAggr(names[saggrN]).val
		var keys = Object.keys(saggr);
		for (var keyN = 0; keyN < keys.length; keyN++) {
			console.print(onStore ? store.name : "default_SA_Base" + " : " + names[saggrN] + " : " + keys[keyN] + " : " +  typeof(saggr[keys[keyN]]) +  " : " + saggr[keys[keyN]] + "\n");
		}
	}
}

//#- `qm.getAllStreamAggrVals(store)` -- returns a JSON where keys are stream aggregate names and values are their corresponding JSON values
qm.getAllStreamAggrVals = function (store) {
    var onStore = arguments.length > 0
    if (!onStore) {
        var names = qm.getStreamAggrNames();
    } else {
        var names = store.getStreamAggrNames();
    }
    var result = {};
    for (var saggrN = 0; saggrN < names.length; saggrN++) {
        result[names[saggrN]] = onStore ? store.getStreamAggr(names[saggrN]).val : qm.getStreamAggr(names[saggrN]).val;
        
    }
    return result;
}

//#- `dir()` -- prints all global variables
//#- `dir(obj, printVals, depth, width, prefix, showProto)` -- recursively prints all keys of object `obj` as well as the keys of `obj.__proto__` (if `showProto` is true, default is false). 
//#   Parameter `printVals` (boolean, default false) prints values if `true` and type if `false`. Depth of recursion is controlled by `depth` (integer, default 1), width is controlled by `width` (integer, default 50). Every line starts with string `prefix`.
function dir(obj, printVals, depth, width, prefix, showProto) {
    // dir();
    if (arguments.length == 0) {
        var globalNames = process.getGlobals();
        for (var nameN = 0; nameN < globalNames.length; nameN++) {
            dir(eval(globalNames[nameN]), false, -1, 100, globalNames[nameN], false);
        }
    }
    printVals = typeof printVals !== 'undefined' ? printVals : false;
    depth = typeof depth !== 'undefined' ? depth : 1;
    width = typeof width !== 'undefined' ? width : 50;
    prefix = typeof prefix !== 'undefined' ? prefix : "";
    showProto = typeof showProto !== 'undefined' ? showProto : false;
    if (depth === parseInt(depth)) {
        if (depth == -1) {
            console.print(prefix + " - ");
            console.println("(" + typeof obj + ")");
        }
        if (depth > 0) {
            if (typeof obj == 'object' && obj != null) {
                var keys = Object.keys(obj);
                var numOwn = keys.length;
                if (showProto) {
                    if (typeof obj.__proto__  !== 'undefined' && obj.__proto__  !== null)
                    keys = keys.concat(Object.getOwnPropertyNames(obj.__proto__));
                }
                for (var keyN = 0; keyN < Math.min(width, keys.length) ; keyN++) {
                    console.print(prefix + "." + keys[keyN] + " - "); if (keyN >= numOwn) { console.print("__proto__ - ");}
                    if (printVals) {
                        if (typeof obj[keys[keyN]] == 'string') {
                            console.print("\""); console.print(obj[keys[keyN]]); console.println("\"");
                        } else if (typeof obj[keys[keyN]] == 'boolean') {
                            console.println(obj[keys[keyN]] ? "true" : "false");
                        } else {
                            console.println(obj[keys[keyN]]);
                        }
                    } else {
                        console.println("(" + typeof obj[keys[keyN]] + ")");
                    }
                    dir(obj[keys[keyN]], printVals, depth - 1, width,  prefix + "." + keys[keyN], showProto);
                }                
            }
        }
    }
}

//#- `printj(obj)` -- prints json (converts obj to json using toJSON if necessary)
function printj(obj) {
    try {
        console.println(JSON.stringify(obj));
    } catch (exception) {
        console.println(JSON.stringify(obj.toJSON()))
    }
}

///////////////////////////////////////// DEPRECATED
qm.addStreamAggr = function (param) {
    console.log("Warning: qm.addStreamAggr is deprecated 4.8.2014. Use qm.newStreamAggr");
    return qm.newStreamAggr(param);
}


///////////////////////////////////////// DEPRECATED
function jsonp(req, res, data) {
    console.log("Warning: jsonp is deprecated 1.7.2014, please use http.jsonp instead");
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
    console.log("Warning: isObject is deprecated 1.7.2014, please use require('utilities.js').isObject instead");
    return require("utilities.js").isObject(arg);
}

function isArray(arg) {
    console.log("Warning: isObject is deprecated 1.7.2014, please use require('utilities.js').isArray instead");
    return require("utilities.js").isArray(arg);
}

function isNumber(n) {
    console.log("Warning: isObject is deprecated 1.7.2014, please use require('utilities.js').isNumber instead");
    return require("utilities.js").isNumber(n);
}

function isString(s) {
    console.log("Warning: isObject is deprecated 1.7.2014, please use require('utilities.js').isString instead");
    return require("utilities.js").isString(s);
}

function ifNull(val, defVal) {
    console.log("Warning: isObject is deprecated 1.7.2014, please use require('utilities.js').ifNull instead");
    return require("utilities.js").ifNull(val, defVal);
}

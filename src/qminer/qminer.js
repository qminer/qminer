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
// process wrapper 
var process = function() {
	var _obj = {};
	_obj.isArg = function(arg) {
        for (var i = 0; i < qm.args.length; i++) {
            if (qm.args[i] == arg) { return true; }
		}
		return false;
	}	

	return _obj;
}();

//////////////////////////////////////////
// Console 

// Start interactive console
console.start = function () {
    while (true) {        
        try {
            console.log("" + eval(console.getln()));
        } catch (err) {
            console.log("Error: " + err.message);
        }
    }
}

//////////////////////////////////////////
// QMiner 

// loading data into stores
qm.load = function() {
	var _obj = {};
	
    // read file line-by-line, parse each line to json and add to store
    // only do this for the first limit lines, or all if limit == -1
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
        console.log("Loaded " + count + " recores to " + store.name);
		return count;
	}

	// read file line-by-line, parse each line to json and add to store
	_obj.jsonFile = function (store, file) {
        return _obj.jsonFileLimit(store, file, -1);
    }
    
    return _obj;
}();

//////////////////////////////////////////
// FileSystem

// read file and return it as string
fs.readFile = function (fileName) {
    var fin = fs.openRead(fileName);
    var out = ""; 
    while (!fin.eof) { 
        out = out + fin.getNextLn() + "\n"; 
    }
    return out;
};

//////////////////////////////////////////
// HTTP 

// shortcuts
http.onGet = function (path, callback) {
    http.onRequest(path, "GET", callback);
}

http.onPost = function (path, callback) {
    http.onRequest(path, "POST", callback);
}

http.onPut = function (path, callback) {
    http.onRequest(path, "PUT", callback);
}

http.onDelete = function (path, callback) {
    http.onRequest(path, "DELETE", callback);
}

http.onPatch = function (path, callback) {
    http.onRequest(path, "PATCH", callback);
}

// packaging reply as jsonp when callback parameter is provided in URL
http.jsonp = function (req, res, data) {
    // convert to string
    var dataStr;
    try {
        dataStr = JSON.stringify(data)
    } catch (err) {
        res.setStatusCode(500);
        res.setContentType("text/plain");
        res.send("error stringifying");
    }
    // return
    res.setStatusCode(200);
    res.setContentType("application/json");
    if (req.args.callback) {           
        res.send(req.args.callback + "(" + dataStr + ");");
    } else {
        res.send(dataStr);
    }
}

function jsonp(req, res, data) {
    console.log("Warning: jsonp is deprecated, please use http.jsonp instead");
    http.jsonp(req, res, data);
}

//////////////////////////////////////////
// Hash table
function hashTable() {
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

//////////////////////////////////////////
// Adds round10, floor10 and ceil10 to Math object.
// Taken from https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Math/round
(function(){
	/**
	 * Decimal adjustment of a number.
	 *
	 * @param	{String}	type	The type of adjustment.
	 * @param	{Number}	value	The number.
	 * @param	{Integer}	exp		The exponent (the 10 logarithm of the adjustment base).
	 * @returns	{Number}			The adjusted value.
	 */
	function decimalAdjust(type, value, exp) {
		// If the exp is undefined or zero...
		if (typeof exp === 'undefined' || +exp === 0) {
			return Math[type](value);
		}
		value = +value;
		exp = +exp;
		// If the value is not a number or the exp is not an integer...
		if (isNaN(value) || !(typeof exp === 'number' && exp % 1 === 0)) {
			return NaN;
		}
		// Shift
		value = value.toString().split('e');
		value = Math[type](+(value[0] + 'e' + (value[1] ? (+value[1] - exp) : -exp)));
		// Shift back
		value = value.toString().split('e');
		return +(value[0] + 'e' + (value[1] ? (+value[1] + exp) : exp));
	}

	// Decimal round
	if (!Math.round10) {
		Math.round10 = function(value, exp) {
			return decimalAdjust('round', value, exp);
		};
	}
	// Decimal floor
	if (!Math.floor10) {
		Math.floor10 = function(value, exp) {
			return decimalAdjust('floor', value, exp);
		};
	}
	// Decimal ceil
	if (!Math.ceil10) {
		Math.ceil10 = function(value, exp) {
			return decimalAdjust('ceil', value, exp);
		};
	}

})();


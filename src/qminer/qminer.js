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
// QMiner JavaScript addons
qm.load = function() {
	var _obj = {};
	
	// read file line-by-line, parse each line to json and add to store
	_obj.jsonFile = function (store, file) {
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
					console.say("  " + count + " records"); 
				}
			} catch (err) {
				console.say("Error parsing [" + line + "]: " + err)
			}
		}
		return count;
	}
	return _obj;
}();

//////////////////////////////////////////
// FileSystem addons
fs.readFile = function (fileName) {
    var fin = fs.openRead(fileName);
    var out = ""; 
    while (!fin.eof) { 
        out = out + fin.getNextLn() + "\n"; 
    }
    return out;
};

//////////////////////////////////////////
// HTTP addons
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
// packaging reply as jsonp when callback is provided
function jsonp(req, res, data) {
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

//////////////////////////////////////////
// is parameter an object
function isObject(arg) {
    if (arg) {
        if ((typeof arg) == "object") {
            //TODO: check if it's not array
            return true;
        } else {
            return false;
        }
    }
    // if no filter, then always true
    return false;
}

//////////////////////////////////////////
// is parameter an array
function isArray(arg) {
    if (arg) {
        if ((typeof arg) == "object") {
            //TODO: check if it's some other object
            return true;
        } else {
            return false;
        }
    }
    // if no filter, then always true
    return false;
}

//////////////////////////////////////////
// is parameter a number
function isNumber(n) {
    return (Object.prototype.toString.call(n) === '[object Number]' 
			|| Object.prototype.toString.call(n) === '[object String]') 
		   &&!isNaN(parseFloat(n)) && isFinite(n.toString().replace(/^-/, ''));
}

//////////////////////////////////////////
// checks if null, and returns default value
function ifNull(val, defVal) {
    return val ? val : defVal
}
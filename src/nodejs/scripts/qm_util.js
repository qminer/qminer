/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
//////////////////////////////////////////
//#- `utilities = require('utilities.js')` -- imports utilities library to variable `utilities`
//#- `bool = utilities.isObject(arg)` -- is parameter an object?

var fs = require('fs');;
var Lazy = require('lazy');

exports.isObject = function (arg) {
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
//#- `bool = utilities.isArray(arg)` -- is parameter an array?
exports.isArray = function (arg) {
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
//#- `bool = utilities.isInArray(array, value)` -- is element in an array?
exports.isInArray = function (array, value) {
    return array.indexOf(value) != -1;
}

//#- `bool = utilities.isNaNInArray(array)` -- returns `true` if one of elements in array is NaN?
exports.isNaNInArray = function (array) {
    for (var i = 0; i < array.length; i++) {
        if (isNaN(array[i])) { return true; }
    }
    return false;
}

//////////////////////////////////////////
//#- `bool = utilities.isNumber(n)` -- is `n` a number?
exports.isNumber = function (n) {
    return (Object.prototype.toString.call(n) === '[object Number]' 
			|| Object.prototype.toString.call(n) === '[object String]') 
		   &&!isNaN(parseFloat(n)) && isFinite(n.toString().replace(/^-/, ''));
}

//////////////////////////////////////////
//#- `bool = utilities.isString(s)` -- is `s` a string?
exports.isString = function (s) {
    return (Object.prototype.toString.call(s) === '[object String]' || (typeof s) === "string");
}

//////////////////////////////////////////
//#- `returnVal = utilities.ifNull(val, defVal)` -- checks if `val` is null and returns default value `defVal`
exports.ifNull = function (val, defVal) {
    return val ? val : defVal
}

/////// STOPWATCH
//#- `sw = utilities.newStopWatch()` -- creates a stop watch object `sw`
exports.newStopWatch = function () {
    return new exports.clsStopwatch();
}
exports.clsStopwatch = function () {
    // Private vars
    var startAt = 0;	// Time of last start / resume. (0 if not running)
    var lapTime = 0;	// Time on the clock when last stopped in milliseconds
    var now = function () {
        return (new Date()).getTime();
    };
    // Public methods    
    // Start or resume
    //#- `sw.start()` -- starts the stopwatch
    this.start = function () {
        startAt = startAt ? startAt : now();
    };
    // Stop or pause
    //#- `sw.stop()` -- stops the stopwatch
    this.stop = function () {
        // If running, update elapsed time otherwise keep it
        lapTime = startAt ? lapTime + now() - startAt : lapTime;
        startAt = 0; // Paused
    };
    // Reset
    //#- `sw.reset()` -- resets 
    this.reset = function () {
        lapTime = startAt = 0;
    };
    // Duration
    //#- `num = sw.time()` -- returns unix epoch time in milliseconds
    this.time = function () {
        return lapTime + (startAt ? now() - startAt : 0);
    };
    // Console say duration
    //#- `sw.saytime(message)` -- displays elpased time from tic
    this.saytime = function (msg) {
        if (typeof msg == "undefined") {
            msg = "";
        }
        console.say(msg + ": elapsed " + this.formatTime(this.time()));
    };
    //#- `sw.tic()` -- resets and starts the stop watch
    this.tic = function () {
        this.reset();
        this.start();
    };
    //#- `sw.toc(str)` -- displays time from tic and message `str`
    this.toc = function (msg) {
        this.saytime(msg);
    };   
    this.pad = function (num, size) {
        var s = "0000" + num;
        return s.substr(s.length - size);
    };
    this.formatTime = function (time) {
        var h = m = s = ms = 0;
        var newTime = '';

        h = Math.floor(time / (60 * 60 * 1000));
        time = time % (60 * 60 * 1000);
        m = Math.floor(time / (60 * 1000));
        time = time % (60 * 1000);
        s = Math.floor(time / 1000);
        ms = time % 1000;

        newTime = this.pad(h, 2) + ':' + this.pad(m, 2) + ':' + this.pad(s, 2) + '.' + this.pad(ms, 3);
        return newTime;
    };
};

//#- `bool = utilities.arraysIdentical(arr, arr2)` -- `bool` is true if array `arr` is identical to array `arr2`
exports.arraysIdentical = function(a, b) {
    var len = a.length;
    if (a.length != b.length) return false;
    for (var elN = 0; elN < len; elN++) {
        if (a[elN] !== b[elN]) return false;
    }
    return true;
};

//#- `string = utilities.numberWithCommas(number)` -- format number 1234 to 1,234
exports.numberWithCommas = function (x) {
    return x.toString().replace(/\B(?=(\d{3})+(?!\d))/g, ",");
}

/**
 * Reads a CSV file, calling a callback function on each line.
 * 
 * @param {object} opts
 * @property {string} opts.file - the file name
 * @property {string} opts.delimiter - CSV cell delimiter
 * @property {function} onLine - a callback(err, line) function called on each line
 * @property {function} onEnd - a callback function called when finished. The function takes no parameters.
 */
exports.readCsvLines = function (opts) {
	// Check to see if the delimiter is defined. If not, then default to comma.
	if (opts.file == null) throw 'Filename missing!';
	if (opts.delimiter == null) opts.delimiter = ',';
	if (opts.onLine == null) throw 'Line callback missing!'
	if (opts.onEnd == null) opts.onEnd = function () {}
	
	var fileName = opts.file;
	var strDelimiter = opts.delimiter;
	
	var lineCb = opts.onLine;
	var endCb = opts.onEnd;
	    		
    // Create a regular expression to parse the CSV values.
    var objPattern = new RegExp((
            // Delimiters.
            "(\\" + strDelimiter + "|\\r?\\n|\\r|^)" +
            // Quoted fields.
            "(?:\"([^\"]*(?:\"\"[^\"]*)*)\"|" +
            // Standard fields.
            "([^\"\\" + strDelimiter + "\\r\\n]*))"
        ), "gi");
    
    new Lazy(fs.createReadStream(fileName)).lines.map(String).forEach(function (line) {
    	try {                                
            var arrData = [ ];
            // Create an array to hold our individual pattern matching groups.
            var arrMatches = null;
            // Keep looping over the regular expression matches until we can no longer find a match.
            while (arrMatches = objPattern.exec(line)){
                // Let's check to see which kind of value we captured (quoted or unquoted).
                var strMatchedValue = arrMatches[2] ?
                    // We found a quoted value. When we capture this value, unescape any double quotes.
                    arrMatches[2].replace(new RegExp( "\"\"", "g" ), "\"") :
                    // We found a non-quoted value.
                    arrMatches[3];
                // Now that we have our value string, let's add it to the data array.
                arrData.push(strMatchedValue);
            }
                            
            // Return the parsed data.
            lineCb(null, arrData);
        } catch (err) {
        	lineCb(err);
        }
    }).on('end', function () {
    	endCb();
    });
}
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
//#- `utilities = require('utilities.js')` -- imports utilities library to variable `utilities`
//#- `bool = utilities.isObject(arg)` -- is parameter an object?
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

//////////////////////////////////////////
//#- `hashTable = utilities.newHashTable()` -- creates a hash table
exports.newHashTable = function () {
    return new exports.hashTable();
}
exports.hashTable = function () {
    this._data = new Object();
    //#  - `strArr = hashTable.keys` -- array of keys (strings)
    this.keys = new Array();
    //#  - `array = hashTable.vals` -- array of values
    this.vals = new Array();
    //#  - `hashTable.put(key)` -- add a key
    this.put = function (key) { this._data[key] = ""; this.keys.push(key); }
    //#  - `hashTable.put(key, dat)` -- add key-dat
    this.put = function (key, dat) { this._data[key] = dat; this.keys.push(key); this.vals.push(dat); }
    //#  - `bool = hashTable.contains(key)` -- does the table contain the key?
    this.contains = function (key) { return this._data.hasOwnProperty(key); }
    //#  - `dat = hashTable.get(key)` -- get data
    this.get = function (key) { return this._data.hasOwnProperty(key) ? this._data[key] : null; }
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

// checks if two JS arrays are identical
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

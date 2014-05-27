//////////////////////////////////////////
// is parameter an object
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
// is parameter an array
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
// is element in an array
exports.isInArray = function (array, value) {
    return array.indexOf(value) != -1;
}

//////////////////////////////////////////
// is parameter a number
exports.isNumber = function (n) {
    return (Object.prototype.toString.call(n) === '[object Number]' 
			|| Object.prototype.toString.call(n) === '[object String]') 
		   &&!isNaN(parseFloat(n)) && isFinite(n.toString().replace(/^-/, ''));
}

//////////////////////////////////////////
// is parameter a string
exports.isString = function (s) {
    return (Object.prototype.toString.call(s) === '[object String]' || (typeof s) === "string");
}

//////////////////////////////////////////
// checks if null, and returns default value
exports.ifNull = function (val, defVal) {
    return val ? val : defVal
}

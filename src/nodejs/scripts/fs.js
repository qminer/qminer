/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
// typical use case: pathPrefix = 'Release' or pathPrefix = 'Debug'. Empty argument is supported as well (the first binary that the bindings finds will be used)
module.exports = exports = function (pathPrefix) {
    pathPrefix = pathPrefix || '';
//    exports = require('bindings')(pathPrefix + '/la.node');
    exports = require('bindings')(pathPrefix + '/qm.node').fs;

    //!STARTJSDOC

    /**
     * Reads a buffer, containing a CSV file, line by line and calls a callback for each line.
     * The callback function accepts an array with the values of the current line.
     *
     * @param {Buffer} buffer - the Node.js buffer
     * @param {Object} opts - options parameter
     * @param {function} opts.onLine - a callback that gets called on each line (for example: function (lineArr) {})
     * @param {function} opts.onEnd - a callback that gets returned after all the lines have been read
     * @param {function} opts.onError - a callback that gets called if an error occurs
     * @param {String} opts.delimiter - the delimiter used when parsing
     * @param {Number} opts.lineLimit - the maximum number of lines read
     * @param {Number} opts.skipLines - the number of lines that should be skipped before first calling the callback
     */
    exports.readCsvLines = function (buff, opts) {
    	if (opts.delimiter == null) opts.delimiter = ',';
    	if (opts.onLine == null) throw 'Line callback missing!';
    	if (opts.onEnd == null) opts.onEnd = function () {}
    	if (opts.onError == null) opts.onError = function () {}
    	if (opts.lineLimit == null) opts.lineLimit = Infinity;
    	if (opts.skipLines == null) opts.skipLines = 0;
    	
    	var strDelimiter = opts.delimiter;
    	
    	var lineCb = opts.onLine;
    	
    	// Create a regular expression to parse the CSV values.
        var objPattern = new RegExp((
                // Delimiters.
                "(\\" + strDelimiter + "|\\r?\\n|\\r|^)" +
                // Quoted fields.
                "(?:\"([^\"]*(?:\"\"[^\"]*)*)\"|" +
                // Standard fields.
                "([^\"\\" + strDelimiter + "\\r\\n]*))"
            ), "gi");
    	
    	var i = 0;
    	exports.readLines(buff, function (line) {
    			// TODO count the number of times that the quote occurred 
    			// if it is an odd number of times, then the line hasn't ended
    			
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
    	            if (i >= opts.skipLines)
    	            	lineCb(arrData);
    	            
    	            return ++i < opts.lineLimit;
    			} catch (e) {
    				opts.onError(e);
    				return true;
    			}
    		}, opts.onEnd, opts.onError);
    }

    //!ENDJSDOC

    return exports;
}

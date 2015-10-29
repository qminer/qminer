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

    function processCsvLine(opts) {
    	if (opts.delimiter == null) opts.delimiter = ',';
    	if (opts.onLine == null) throw 'Line callback missing!';
    	if (opts.onEnd == null) opts.onEnd = function () {}
    	if (opts.lineLimit == null) opts.lineLimit = Infinity;
    	if (opts.skipLines == null) opts.skipLines = 0;
    	if (opts.quote == null) opts.quote = '"';
    	
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
        
        var quoteRegex = new RegExp(opts.quote, 'g');
        
        var i = 0;
    	var line = '';
    	
    	return function (batch) {
    		line += batch;
    		
    		// if the number of delimiters is odd => the line hasn't ended, but a string field contained a line ending
    		if ((line.match(quoteRegex) || []).length % 2 == 1) {
    			return true;
    		}
			
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
	            
	            i++;
	            line = '';
	            
	            // Return the parsed data.
	            if (i > opts.skipLines) {
	            	lineCb(arrData);
	            }
	            
	            return ++i < opts.lineLimit;
			} catch (e) {
				opts.onEnd(e);
				return false;	// stop the loop if an error occurs
			}
    	}
    }
    
    function processCsvBatch(opts) {
    	var processLine = processCsvLine(opts);
    	
    	return function (e, batch) {
    		if (e != null) {
    			opts.onEnd(e);
    			return;
    		}
    		for (var i = 0; i < batch.length; i++) {
    			processLine(batch[i]);
    		}
    		
    		opts.onEnd();
    	}
    }
    
    /**
     * Reads a buffer, containing a CSV file, line by line and calls a callback for each line.
     * The callback function accepts an array with the values of the current line.
     *
     * @param {Buffer} buffer - the Node.js buffer
     * @param {Object} opts - options parameter
     * @param {function} opts.onLine - a callback that gets called on each line (for example: function (lineArr) {})
     * @param {function} opts.onEnd - a callback that gets returned after all the lines have been read
     * @param {String} opts.delimiter - the delimiter used when parsing
     * @param {Number} opts.lineLimit - the maximum number of lines read
     * @param {Number} opts.skipLines - the number of lines that should be skipped before first calling the callback
     */
    exports.readCsvLines = function (fin, opts) {
    	exports.readLines(fin, processCsvLine(opts), opts.onEnd);
    }
    
    exports.readCsvLinesAsync = function (fin, opts) {
    	 var readerOpts = { offset: opts.skipLines };
         if (opts.lineLimit != null) readerOpts.limit = opts.lineLimit;
         
         delete opts.skipLines;
         delete opts.lineLimit;
         
         exports.readLinesAsync(fin, readerOpts, processCsvBatch(opts));
    }

    //!ENDJSDOC

    return exports;
}

/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
 
module.exports = exports = function (pathQmBinary) {    
    var qm = require(pathQmBinary); // This loads only c++ functions of qm
    exports = qm.fs;
    
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
    
    /**
     * Reads a buffer, containing a CSV file, line by line and calls a callback for each line.
     * As specified in CSV format standard defined in [RFC 4180]{@link https://tools.ietf.org/html/rfc4180}
     * double-quotes (") can be used as escape characters. If a double-quote appears in a field, the field nust be
     * enclosed in double-quotes and the double-quote appearing inside a field must be escaped by preceding it with
     * another double quote.
     * The callback function accepts an array with the values of the current line.
     * @param {Buffer} buffer - The Node.js buffer.
     * @param {Object} opts - Options parameter.
     * @param {function} opts.onLine - A callback that gets called on each line (for example: `function (lineArr) {}`).
     * @param {function} opts.onEnd - A callback that gets returned after all the lines have been read.
     * @param {String} opts.delimiter - The delimiter used when parsing.
     * @param {Number} opts.lineLimit - The maximum number of lines read.
     * @param {Number} opts.skipLines - The number of lines that should be skipped before first calling the callback.
     * @example
     * // import fs module                                                                                                                                   
     * let fs = require('qminer').fs;                                                                                                                        
     * // create a file and write some lines                                                                                                                 
     * let fout = fs.openWrite('test.csv');                                                                                                                  
     * fout.write('name,movie\nGeorge Clooney,"O Brother, Where Art Thou?"\n"Sylvester ""Sly"" Stallone",Expendables');                                      
     * fout.close();                                                                                                                                         
     * // open the file in read mode                                                                                                                         
     * let fin = fs.openRead('test.csv');                                                                                                                    
     * // prepare callbacks for csv parsing                                                                                                                  
     * // count the lines and for each line output the parsed cells                                                                                          
     * let nLines = 0;                                                                                                                                       
     * function onLine(lineVals) {                                                                                                                           
     *     nLines += 1;                                                                                                                                      
     *     console.log(lineVals);                                                                                                                            
     *     return true;                                                                                                                                      
     * }                                                                                                                                                     
     * // at the end output the number of lines                                                                                                              
     * function onEnd(err) {                                                                                                                                 
     *     if (err) { console.log("Error:", err); }                                                                                                          
     *     console.log("Number of lines", nLines);                                                                                                           
     * }                                                                                                                                                     
     * // parse the csv files                                                                                                                                
     * fs.readCsvLines(fin, {                                                                                                                                
     *     "onLine": onLine,                                                                                                                                 
     *     "onEnd": onEnd                                                                                                                                    
     * });                                                                                                                                                   
     */
    exports.readCsvLines = function (fin, opts) {
    	exports.readLines(fin, processCsvLine(opts), opts.onEnd);
    }
    
    /**
     * Reads json that was serialized using `fs.FOut.writeJson`.
     * @returns {Object} Json object.
     * @example
     * // import fs module
     * var fs = require('qminer').fs;
     * // create and save a json and then read it using the readJson method
     */
    exports.FIn.prototype.readJson = function () {
    	var str = this.readString();
    	return JSON.parse(str);
    }
    
    /**
     * Saves json object, which can be read by `fs.FIn.readJson`.
     * @returns {Object} obj - Json object to write.
     * @returns {module:fs.FOut} Self.
     * @example
     * // import fs module
     * var fs = require('qminer').fs;
     * // create and save a json using the writeJson method
     */
    exports.FOut.prototype.writeJson = function (json) {
    	var str = JSON.stringify(json);
    	this.writeBinary(str);
    	return this;
    }

    //!ENDJSDOC

    return exports;
}

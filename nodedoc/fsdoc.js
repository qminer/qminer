/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
/**
* File-system module.
* @module fs
* @example
* // import module
* var fs = require('qminer').fs;
* // open file in write mode
* var fout = fs.openWrite('file.txt');
* // write sync and close
* fout.writeLine('example text');
* fout.close();
* // open file in read mode
* var fin = fs.openRead('file.txt');
* // read a line
* var str = fin.readLine();
*/
/**
    * Open file in read mode and return file input stream.
    * @param {string} fileName - File name.
    * @returns {module:fs.FIn} Input stream.
    * @example
    * // import fs module
    * var fs = require('qminer').fs;
    * // open file to write
    * var fout = fs.openWrite('read_text.txt');
    * // write to file
    * fout.write('This is awesome!');
    * // close the stream
    * fout.close();
    * // open file to read
    * var fin = fs.openRead('read_text.txt');
    */
 exports.openRead = function(fileName) { return Object.create(require('qminer').fs.FIn.prototype); }
/**
    * Open file in write mode and return file output stream.
    * @param {string} fileName - File name.
    * @returns {module:fs.FOut} Output stream.
    * @example
    * // import fs module
    * var fs = require('qminer').fs;
    * // open file to write
    * var fout = fs.openWrite('write_text.txt');
    * // close the stream
    * fout.close();
    */
 exports.openWrite = function(fileName) { return Object.create(require('qminer').fs.FOut.prototype); }
/**
    * Open file in append mode and return file output stream.
    * @param {string} fileName - File name.
    * @returns {module:fs.FOut} Output stream.
    * @example
    * // import fs module
    * var fs = require('qminer').fs;
    * // open file to write
    * var fout = fs.openWrite('append_text.txt');
    * // close the stream
    * fout.close();
    * // open file in append mode
    * var foutAppend = fs.openAppend('append_text.txt');
    * // close the stream
    * foutAppend.close();
    */
 exports.openAppend = function(fileName) { return Object.create(require('qminer').fs.FOut.prototype); }
/**
    * Checks if the file exists.
    * @param {string} fileName - File name.
    * @returns {boolean} True if file exists.
    * @example
    * // import fs module
    * var fs = require('qminer').fs;
    * // check if a file exists
    * fs.exists('text.txt');
    */
 exports.exists = function(fileName) { return false; }
/**
    * Copies a file.
    * @param {string} source - Source file name.
    * @param {string} dest - Destination file name.
    * @example
    * // import fs module
    * var fs = require('qminer').fs;
    * // open file to write
    * var fout = fs.openWrite('text.txt');
    * // close the stream
    * fout.close();
    * // copy the file
    * // var destination = fs.copy('text.txt', 'copy.txt');
    */
 exports.copy = function(source, dest) { return ""; }
/**
    * Moves a file.
    * @param {string} source - Source file name.
    * @param {string} dest - Destination file name.
    * @example
    * // import fs module
    * var fs = require('qminer').fs;
    * // open file to write
    * var fout = fs.openWrite('text.txt');
    * // close the stream
    * fout.close();
    * // move the file
    * // var destination = fs.move('text.txt', 'move.txt');
    */
 exports.move = function(source, dest) { return ""; }
/**
    * Deletes a file.
    * @param {string} fileName - File name.
    * @returns {boolean} True if delete succeeded.
    * @example
    * // import fs module
    * var fs = require('qminer').fs;
    * // open file to write
    * var fout = fs.openWrite('delete.txt');
    * // close the stream
    * fout.close();
    * // delete the file
    * var destination = fs.del('delete.txt');
    */
 exports.del = function(fileName) { return false; }
/**
    * Renames a file.
    * @param {string} source - Source file name.
    * @param {string} dest - Destination file name.
    * @example
    * // import fs module
    * var fs = require('qminer').fs;
    * // open file to write
    * var fout = fs.openWrite('text.txt');
    * // close the stream
    * fout.close();
    * // rename the file
    * if (fs.exists('rename.txt')) {
    *    fs.del('rename.txt');
    * }
    * var destination = fs.rename('text.txt', 'rename.txt');
    */
 exports.rename = function(source, dest) { return ""; }
/**
    * @typedef {Object} FileInfo
    * Information about the file.
    * @property  {string} createTime - Create time.
    * @property  {string} lastAccessTime - Last access time.
    * @property  {string} lastWriteTime - Last write time.
    * @property  {number} size - File size in bytes.
    */
/**
    * Returns the file info.
    * @param {string} fileName - File name.
    * @returns {module:fs~FileInfo} File info object.
    * @example
    * // import fs module
    * var fs = require('qminer').fs;
    * // open file to write
    * var fout = fs.openWrite('text.txt');
    * // close the stream
    * fout.close();
    * // get the file info
    * var info = fs.fileInfo('text.txt');
    */
 exports.fileInfo = function(fileName) { return { createTime : "",  lastAccessTime: "", lastWriteTime: "", size: 0 }}
/**
    * Creates a folder.
    * @param {string} dirName - Folder name.
    * @returns {boolean} True if succeeded.
    * @example
    // import fs module
    * var fs = require('qminer').fs;
    * // create a folder
    * var makeFolder = fs.mkdir('folder');
    */
 exports.mkdir = function(dirName) { return false; }
/**
    * Removes a folder.
    * @param {string} dirName - Folder name.
    * @returns {boolean} True if succeeded.
    * @example
    // import fs module
    * var fs = require('qminer').fs;
    * // create a folder
    * var makeFolder = fs.mkdir('folder');
    * // delete folder
    * if (makeFolder) {
    *    fs.rmdir('folder');
    * }
    */
 exports.rmdir = function(dirName) { return false; }
/**
    * Returns a list fo files in the folder.
    * @param {string} dirName - Folder name.
    * @param {string} [fileExtension] - Results are filtered by file extension.
    * @param {boolean} [recursive=false] - Recursively searches for file names if true.
    * @returns {Array.<string>} Array of file names.
    * @example
    * // import fs module
    * var fs = require('qminer').fs;
    * // get the names of all files
    * var fileNames = fs.listFile('./');
    */
 exports.listFile = function(dirName, fileExtension, recursive) { return ['']; }
/**
     * Reads a buffer line by line and calls a callback for each line.
     * @param {String | module:fs.FIn | Buffer} buffer - Name of the file, input stream of a Node.js buffer.
     * @param {function} onLine(line) - A callback that gets called on each line (for example: `function (line) {}`).
     *   Function must return `true` to continue reading, else reading is stoped and `onEnd` is called.
     * @param {function} onEnd(err) - A callback that gets returned after all the lines have been read or
     *   function `onLine` returned `false`. If error was due to exception, the exception is provided in `err`.
     * @example
     * // import fs module
     * var fs = require('qminer').fs;
     * // create a file and write some lines
     * var fout = fs.openWrite('poem.txt');
     * fout.write('I dig,\nYou dig,\nHe digs,\nShe digs,\nWe dig,\nThey dig.\n It\'s not a beautiful poem, but it\'s deep.');
     * fout.close();
     * // open the file in read mode
     * var fin = fs.openRead('poem.txt');
     * // read the file line by line and call functions
     * var numberOfLines = 0;
     * function onLine(line) {
     *     console.log(line);
     *     numberOfLines += 1;
     *     return true;
     * }
     * function onEnd(err) {
     *     if (err) { console.log("Error:", err); }
     *     console.log("Number of lines", numberOfLines);
     * }
     * fs.readLines(fin, onLine, onEnd);
     */
 exports.readLines = function (buffer, onLine, onEnd) {}
/**
    * Input file stream.
    * @classdesc Used for reading files.
    * @class
    * @param {string} fileName - File name.
    * @example
    * // import module
    * var fs = require('qminer').fs;
    * // open file in write mode
    * var fout = fs.openWrite('file.txt');
    * // write sync and close
    * fout.writeLine('example text');
    * fout.close();
    * // open file in read mode
    * var fin = new fs.FIn('file.txt');
    * // read a line
    * var str = fin.readLine();
    */
 exports.FIn = function(fileName) { return Object.create(require('qminer').fs.FIn.prototype); }
/**
    * Peeks a character.
    * @returns {string} Character string.
    * @example
    * // import module
    * var fs = require('qminer').fs;
    * // open file in write mode
    * var fout = fs.openWrite('file.txt');
    * // write sync and close
    * fout.writeLine('example text');
    * fout.close();
    * // open file in read mode
    * var fin = new fs.FIn('file.txt');
    * // peek the next character
    * var char = fin.peekCh();
    */
 exports.FIn.prototype.peekCh= function() { return ''; }
/**
    * Reads a character.
    * @returns {string} Character string.
    * @example
    * // import module
    * var fs = require('qminer').fs;
    * // open file in write mode
    * var fout = fs.openWrite('file.txt');
    * // write sync and close
    * fout.writeLine('example text');
    * fout.close();
    * // open file in read mode
    * var fin = new fs.FIn('file.txt');
    * // get the next character
    * var char = fin.getCh();
    */
 exports.FIn.prototype.getCh= function() { return ''; }
/**
    * Reads a line.
    * @returns {string} Line string.
    * @example
    * // import module
    * var fs = require('qminer').fs;
    * // open file in write mode
    * var fout = fs.openWrite('file.txt');
    * // write sync and close
    * fout.writeLine('example text');
    * fout.close();
    * // open file in read mode
    * var fin = new fs.FIn('file.txt');
    * // get/read a new line
    * var line = fin.readLine();
    */
 exports.FIn.prototype.readLine = function() { return ''; }
/**
    * Reads a string that was serialized using `fs.FOut.writeBinary`.
    * @returns {string} String.
    * @example
    * // import fs module
    * var fs = require('qminer').fs;
    * // read a string that was serialized using fs.FOut.writeBinary
    */
 exports.FIn.prototype.readString = function() { return ''; }
/**
    * True if end of file is detected. Otherwise, false. Type `boolean`.
    * @example
    * // import module
    * var fs = require('qminer').fs;
    * // open file in write mode
    * var fout = fs.openWrite('file.txt');
    * // write sync and close
    * fout.writeLine('example text');
    * fout.close();
    * // open file in read mode
    * var fin = new fs.FIn('file.txt');
    * // check if it's end of the file
    * var eof = fin.eof;
    */
 exports.FIn.prototype.eof = false;
/**
    * Length of input stream. Type `number`.
    * @example
    * // import module
    * var fs = require('qminer').fs;
    * // open file in write mode
    * var fout = fs.openWrite('file.txt');
    * // write sync and close
    * fout.writeLine('example text');
    * fout.close();
    * // open file in read mode
    * var fin = new fs.FIn('file.txt');
    * // get the length of the document
    * var len = fin.length;
    */
 exports.FIn.prototype.length = 0;
/**
    * Reads the whole stream.
    * @returns {string} Content of the file.
    * @example
    * // import module
    * var fs = require('qminer').fs;
    * // open file in write mode
    * var fout = fs.openWrite('file.txt');
    * // write sync and close
    * fout.writeLine('example text');
    * fout.close();
    * // open file in read mode
    * var fin = new fs.FIn('file.txt');
    * // get/read a the whole string
    * var all = fin.readAll();
    */
 exports.FIn.prototype.readAll = function() { return ''; }
/**
    * Closes the input stream.
    * @example
    * // import module
    * var fs = require('qminer').fs;
    * // open file in write mode
    * var fout = fs.openWrite('file.txt');
    * // write sync and close
    * fout.writeLine('example text');
    * fout.close();
    * // open file in read mode
    * var fin = new fs.FIn('file.txt');
    * // close the stream
    * fin.close();
    */
 exports.FIn.prototype.close = function() { }
/**
    * Checks if the input stream is closed.
    * @example
    * // import module
    * var fs = require('qminer').fs;
    * // open file in write mode
    * var fout = fs.openWrite('file.txt');
    * // write sync and close
    * fout.writeLine('example text');
    * fout.close();
    * // open file in read mode
    * var fin = new fs.FIn('file.txt');
    * // check if the stream is closed
    * var check = fin.isClosed();
    */
 exports.FIn.prototype.isClosed = function() { return false; }
/**
    * Output file stream.
    * @classdesc Used for writing files.
    * @class
    * @param {String} fileName - File name.
    * @param {boolean} [append=false] - Append flag.
    * @example
    * // import module
    * var fs = require('qminer').fs;
    * // open file in write mode
    * var fout = new fs.FOut('file.txt');
    * // write a line
    * fout.writeLine('example text');
    * // close
    * fout.close();
    */
 exports.FOut = function(fileName, append) {}
/**
    * Writes a string or number or a JSON object in human readable form.
    * @param {(String | Number | Object)} arg - Argument to write.
    * @returns {module:fs.FOut} Self.
    * @example
    * // import module
    * var fs = require('qminer').fs;
    * // open file in write mode
    * var fout = new fs.FOut('file.txt');
    * // write a string
    * fout.write('example text');
    * // close
    * fout.close();
    */
 exports.FOut.prototype.write = function(arg) { return this; }
/**
    * Writes a string or number or a JSON object in binary form.
    * @param {(String | Number | Object)} str - Argument to write.
    * @returns {module:fs.FOut} Self.
    * @example
    * // import fs module
    * var fs = require('qminer').fs;
    * // save a string in binary form
    */
 exports.FOut.prototype.writeBinary = function(arg) { return this; }
/**
    * Writes a string and adds a new line.
    * @param {String} str - String to write.
    * @returns {module:fs.FOut} Self.
    * @example
    * // import module
    * var fs = require('qminer').fs;
    * // open file in write mode
    * var fout = new fs.FOut('file.txt');
    * // write a line
    * fout.writeLine('example text');
    * // close
    * fout.close();
    */
 exports.FOut.prototype.writeLine = function(str) { return this; }
/**
    * Flushes the output stream.
    * @returns {module:fs.FOut} Self.
    * @example
    * // import module
    * var fs = require('qminer').fs;
    * // open file in write mode
    * var fout = new fs.FOut('file.txt');
    * // write a line
    * fout.writeLine('example text');
    * // flush the stream
    * fout.flush();
    * // close
    * fout.close();
    */
 exports.FOut.prototype.flush = function() { return this; }
/**
    * Closes the output stream.
    * @example
    * // import module
    * var fs = require('qminer').fs;
    * // open file in write mode
    * var fout = new fs.FOut('file.txt');
    * // write a line
    * fout.writeLine('example text');
    * // close
    * fout.close();
    */
 exports.FOut.prototype.close = function() {}


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

    

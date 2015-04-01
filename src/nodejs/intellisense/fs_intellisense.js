//intellisense start
exports = {}; require.modules.qminer_fs = exports;
//intellisense end
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
	* open file in read mode and return file input stream
	* @param {string} fileName - File name.
	* @returns {module:fs.FIn} Input stream.
	*/
 exports.openRead = function(fileName) { return Object.create(require('qminer').fs.FIn.prototype); }
/**
	* open file in write mode and return file output stream
	* @param {string} fileName - File name.
	* @returns {module:fs.FOut} Output stream.
	*/
 exports.openWrite = function(fileName) {}
/**
	* open file in append mode and return file output stream
	* @param {string} fileName - File name.
	* @returns {module:fs.FOut} Output stream.
	*/
 exports.openAppend = function(fileName) {}	
/**
	* checks if the file exists
	* @param {string} fileName - File name.
	* @returns {boolean} True if file exists.
	*/
 exports.exists = function(fileName) {}	
/**
	* copies a file
	* @param {string} source - Source file name.
	* @param {string} dest - Destination file name.
	*/
 exports.copy = function(source, dest) {}	
/**
	* moves a file
	* @param {string} source - Source file name.
	* @param {string} dest - Destination file name.
	*/
 exports.move = function(source, dest) {}
/**
	* deletes a file
	* @param {string} fileName - File name.
	* @returns {boolean} True if delete succeeded.
	*/
 exports.del = function(fileName) {}	
/**
	* renames a file
	* @param {string} source - Source file name.
	* @param {string} dest - Destination file name.
	*/
 exports.rename = function(source, dest) {}
/**
	* Information about the file
	* @typedef {Object} FileInfo 	
	* @property  {string} FileInfo.createTime - Create time.
	* @property  {string} FileInfo.lastAccessTime - Last access time.
	* @property  {string} FileInfo.lastWriteTime - Last write time.
	* @property  {number} FileInfo.size - File size in bytes.	
	*/
/**
	* returns the file info
	* @param {string} fileName - File name.
	* @returns {module:fs~FileInfo} File info object.
	*/
 exports.fileInfo = function(fileName) {}	
/**
	* Creates a folder
	* @param {string} dirName - Folder name.
	* @returns {boolean} True if succeeded.
	*/
 exports.mkdir = function(dirName) {}	
/**
	* Removes a folder
	* @param {string} dirName - Folder name.
	* @returns {boolean} True if succeeded.
	*/
 exports.rmdir = function(dirName) {}
/**
	* Returns a list fo files in the folder
	* @param {string} dirName - Folder name.
	* @param {string} [fileExtension] - Results are filtered by file extension.
	* @param {boolean} [recursive=false] - Recursively searches for file names if true.
	* @returns {string[]} True if succeeded.
	*/
 exports.listFile = function(dirName, fileExtension, recursive) {}
/**
	* Input file stream.
	* @classdesc Used for reading files.
	* @class
	* @param {string} fileName - File name
	* @example
	* // import module
	* var fs = require('qminer').fs;
	* // open file in read mode
	* var fin = new fs.FIn('file.txt');
	* // read a line
	* var str = fin.readLine();
	*/
 exports.FIn = function(fnm) {}	
/**
	* Peeks a character
	* @returns {string} Character string.
	*/
 exports.FIn.prototype.peekCh= function() {}
/**
	* Reads a character
	* @returns {string} Character string.
	*/
 exports.FIn.prototype.getCh= function() {}
/**
	* Reads a line	
	* @returns {string} Line string.
	*/
 exports.FIn.prototype.readLine = function() {}
/**
	* @property {boolean} eof - True if end of file is detected.
	*/
 exports.FIn.prototype.eof = undefined;
/**
	* @property {number} length - Length of input stream.
	*/
 exports.FIn.prototype.length = undefined;
/**
	* Reads the whole stream
	* @returns {string} Content of the file.
	*/
 exports.FIn.prototype.readAll = function() {}
/**
	* Output file stream.
	* @classdesc Used for writing files.
	* @class
	* @param {String} fileName - File name
	* @param {boolean} [append=false] - Append flag
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
	* Writes a string
	* @param {String} str - String to write
	* @returns {module:fs.FOut} Self.
	*/
 exports.FOut.prototype.write = function(str) {}
/**
	* Writes a string and adds a new line
	* @param {String} str - String to write
	* @returns {module:fs.FOut} Self.
	*/
 exports.FOut.prototype.writeLine = function(str) {}
/**
	* Flushes the output stream
	* @returns {module:fs.FOut} Self.
	*/
 exports.FOut.prototype.flush = function() {}
/**
	* Closes the output stream
	*/
 exports.FOut.prototype.close = function() {}

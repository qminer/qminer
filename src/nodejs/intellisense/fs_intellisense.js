/**
 * Copyright (c) 2015, Quintelligence d.o.o.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 */
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
 exports.openWrite = function(fileName) { return Object.create(require('qminer').fs.FOut.prototype); }
/**
	* open file in append mode and return file output stream
	* @param {string} fileName - File name.
	* @returns {module:fs.FOut} Output stream.
	*/
 exports.openAppend = function(fileName) { return Object.create(require('qminer').fs.FOut.prototype); }	
/**
	* checks if the file exists
	* @param {string} fileName - File name.
	* @returns {boolean} True if file exists.
	*/
 exports.exists = function(fileName) { return false; }	
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
 exports.del = function(fileName) { return false; }	
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
 exports.fileInfo = function(fileName) { return { createTime : "",  lastAccessTime: "", lastWriteTime: "", size: 0 }}	
/**
	* Creates a folder
	* @param {string} dirName - Folder name.
	* @returns {boolean} True if succeeded.
	*/
 exports.mkdir = function(dirName) { return false; }	
/**
	* Removes a folder
	* @param {string} dirName - Folder name.
	* @returns {boolean} True if succeeded.
	*/
 exports.rmdir = function(dirName) { return false; }
/**
	* Returns a list fo files in the folder
	* @param {string} dirName - Folder name.
	* @param {string} [fileExtension] - Results are filtered by file extension.
	* @param {boolean} [recursive=false] - Recursively searches for file names if true.
	* @returns {string[]} Array of file names.
	*/
 exports.listFile = function(dirName, fileExtension, recursive) { return ['']; }
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
 exports.FIn.prototype.peekCh= function() { return ''; }
/**
	* Reads a character
	* @returns {string} Character string.
	*/
 exports.FIn.prototype.getCh= function() { return ''; }
/**
	* Reads a line	
	* @returns {string} Line string.
	*/
 exports.FIn.prototype.readLine = function() { return ''; }
/**
	* @property {boolean} eof - True if end of file is detected.
	*/
 exports.FIn.prototype.eof = false;
/**
	* @property {number} length - Length of input stream.
	*/
 exports.FIn.prototype.length = 0;
/**
	* Reads the whole stream
	* @returns {string} Content of the file.
	*/
 exports.FIn.prototype.readAll = function() { return ''; }
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
 exports.FOut.prototype.write = function(str) { return this; }
/**
	* Writes a string and adds a new line
	* @param {String} str - String to write
	* @returns {module:fs.FOut} Self.
	*/
 exports.FOut.prototype.writeLine = function(str) { return this; }
/**
	* Flushes the output stream
	* @returns {module:fs.FOut} Self.
	*/
 exports.FOut.prototype.flush = function() { return this; }
/**
	* Closes the output stream
	*/
 exports.FOut.prototype.close = function() {}

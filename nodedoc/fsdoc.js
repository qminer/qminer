/**
* File-system module.
* @module fs
*/
/**
	* open file in read mode and return file input stream
	* @param {string} fileName - File name.
	* @returns {module:fs.FIn} fin - Input stream.
	*/
 exports.openRead = function(fileName) {}
/**
	* open file in write mode and return file output stream
	* @param {string} fileName - File name.
	* @returns {module:fs.FOut} fout - Output stream.
	*/
 exports.openWrite = function(fileName) {}
/**
	* Input file stream.
	* @classdesc Used for reading files.
	* @class
	* @param {String} fileName - File name
	*/
 exports.FIn = function(fnm) {}	
/**
	* Reads a line	
	* @returns {String} line - Returns a line.
	*/
 exports.FIn.prototype.readLine = function() {}
/**
	* Output file stream.
	* @classdesc Used for writing files.
	* @class
	* @param {String} fileName - File name
	* @param {boolean} [append=false] - Append flag
	*/
 exports.FOut = function(fileName, append) {}	
/**
	* Writes a string
	* @param {String} str - String to write
	* @returns {module:fs.FOut} this - Returns self.
	*/
 exports.FOut.prototype.write = function(str) {}

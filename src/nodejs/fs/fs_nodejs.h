/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef QMINER_FS_NODEJS_H
#define QMINER_FS_NODEJS_H


#include <node.h>
#include <node_buffer.h>
#include <node_object_wrap.h>
#include "base.h"
#include "../nodeutil.h"

///////////////////////////////
/// JavaScript Directory
class TNodeJsFPath {
private:
    /// Canonicalized file path 
    TStr CanonicalFPath; 
public:
    TNodeJsFPath(const TStr& FPath = "./");
    /// Is directory subdirectory of this 
    bool IsSubdir(const TNodeJsFPath& JsFPath) const; 
    /// Directory equals this 
    bool Equals(const TNodeJsFPath& JsFPath) const; 
    /// Get directory name
    const TStr& GetFPath() const { return CanonicalFPath; }
    /// Load list of directories
    static void GetFPathV(const TStrV& FPathV, TVec<TNodeJsFPath>& JsFPathV);
    /// Canonicalize file path 
    static TStr GetCanonicalPath(const TStr& FPath);
};

///////////////////////////////
// NodeJs-Filesystem

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
class TNodeJsFs : public node::ObjectWrap {
public:
    // directories we're allowed to access 
    TVec<TNodeJsFPath> AllowedFPathV;
private:
    TNodeJsFs(const TVec<TNodeJsFPath>& AllowedDirV_ = TVec<TNodeJsFPath>()):
        AllowedFPathV(AllowedDirV_) { }
public:
    static void Init(v8::Handle<v8::Object> exports);
    
private:
	class TReadCsvTask: public TNodeTask {
	private:
		PSIn SIn;
		int Offset;
		int Limit;
		int BatchSize;
		v8::Persistent<v8::Function> OnLine;

	public:
		TReadCsvTask(const v8::FunctionCallbackInfo<v8::Value>& Args);
		~TReadCsvTask();

		v8::Handle<v8::Function> GetCallback(const v8::FunctionCallbackInfo<v8::Value>& Args);
		void Run();
	};

	struct TReadLinesCallback {
		TVec<TStrV> CsvLineV;
		v8::Persistent<v8::Function>* OnLine;
		TReadLinesCallback(const int& BatchSize, v8::Persistent<v8::Function>* _OnLine):
			CsvLineV(BatchSize, 0),
			OnLine(_OnLine) {}
		static void Run(const TReadLinesCallback& Task);
	};

public:

	/**
	* open file in read mode and return file input stream
	* @param {string} fileName - File name.
	* @returns {module:fs.FIn} Input stream.
	*/
	//# exports.openRead = function(fileName) { return Object.create(require('qminer').fs.FIn.prototype); }
    JsDeclareFunction(openRead);
    
	/**
	* open file in write mode and return file output stream
	* @param {string} fileName - File name.
	* @returns {module:fs.FOut} Output stream.
	*/
	//# exports.openWrite = function(fileName) { return Object.create(require('qminer').fs.FOut.prototype); }
    JsDeclareFunction(openWrite);
	
	/**
	* open file in append mode and return file output stream
	* @param {string} fileName - File name.
	* @returns {module:fs.FOut} Output stream.
	*/
	//# exports.openAppend = function(fileName) { return Object.create(require('qminer').fs.FOut.prototype); }	
	JsDeclareFunction(openAppend);
	
	/**
	* checks if the file exists
	* @param {string} fileName - File name.
	* @returns {boolean} True if file exists.
	*/
	//# exports.exists = function(fileName) { return false; }	
    JsDeclareFunction(exists);
	
	/**
	* copies a file
	* @param {string} source - Source file name.
	* @param {string} dest - Destination file name.
	*/
	//# exports.copy = function(source, dest) {}	
    JsDeclareFunction(copy);
	
	/**
	* moves a file
	* @param {string} source - Source file name.
	* @param {string} dest - Destination file name.
	*/
	//# exports.move = function(source, dest) {}
	JsDeclareFunction(move);
	
	/**
	* deletes a file
	* @param {string} fileName - File name.
	* @returns {boolean} True if delete succeeded.
	*/
	//# exports.del = function(fileName) { return false; }	
    JsDeclareFunction(del);
	
	/**
	* renames a file
	* @param {string} source - Source file name.
	* @param {string} dest - Destination file name.
	*/
	//# exports.rename = function(source, dest) {}
    JsDeclareFunction(rename);

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
	//# exports.fileInfo = function(fileName) { return { createTime : "",  lastAccessTime: "", lastWriteTime: "", size: 0 }}	
    JsDeclareFunction(fileInfo);
	
	/**
	* Creates a folder
	* @param {string} dirName - Folder name.
	* @returns {boolean} True if succeeded.
	*/
	//# exports.mkdir = function(dirName) { return false; }	
    JsDeclareFunction(mkdir);
	
	/**
	* Removes a folder
	* @param {string} dirName - Folder name.
	* @returns {boolean} True if succeeded.
	*/
	//# exports.rmdir = function(dirName) { return false; }
    JsDeclareFunction(rmdir);
	
	/**
	* Returns a list fo files in the folder
	* @param {string} dirName - Folder name.
	* @param {string} [fileExtension] - Results are filtered by file extension.
	* @param {boolean} [recursive=false] - Recursively searches for file names if true.
	* @returns {string[]} Array of file names.
	*/
	//# exports.listFile = function(dirName, fileExtension, recursive) { return ['']; }
    JsDeclareFunction(listFile);

    /**
     * Reads a buffer line by line and calls a callback for each line.
     *
     * @param {String|FIn|Buffer} buffer - name of the file, input stream of a Node.js buffer
     * @param {function} onLine - a callback that gets called on each line (for example: function (line) {})
     * @param {function} onEnd - a callback that gets returned after all the lines have been read
     * @param {function} onError - a callback that gets called if an error occurs
     */
    //# exports.readLines = function (buffer, onLine, onEnd, onError) {}
    JsDeclareFunction(readLines);

    JsDeclareAsyncFunction(readCsvAsync, TReadCsvTask);
};

///////////////////////////////
// NodeJs-FIn    
class TNodeJsFIn : public node::ObjectWrap {
	friend class TNodeJsUtil;
// Class implementation requirements:
// Node framework: 
//    -implement Init
//    -make sure Init is called from somewhere (in this case from init function, registered with NODE_MODULE(...))
// creating object from C++ (using TNodeJsUtil::NewJsInstance(TClass* Obj))
//    -define Constructor member
//    -define GetClassId method
//    -set Constructor callback to TNodeJsUtil::_NewCpp (doesn't create a new pointer to the wrapper) and inherit from template
// creating from JS using 'new' 
//	  -implement NewFromArgs (parses arguments and returns pointer to wrapper)
//	  -define GetClassId method
//    -set template callback to TNodeJsUtil::_NewJs (this creates a new pointer to the wrapper)
//    -attach template function to exports in Init function
private:
	static v8::Persistent<v8::Function> Constructor;
public:
	static void Init(v8::Handle<v8::Object> Exports);
	static const TStr GetClassId() { return "FIn"; }

	// wrapped C++ object
	PSIn SIn;
	// C++ constructor
	TNodeJsFIn(const TStr& FNm) : SIn(TZipIn::NewIfZip(FNm)) { }
	TNodeJsFIn(const PSIn& _SIn) : SIn(_SIn) { }
private:	
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
	//# exports.FIn = function(fnm) {}	
	// parses arguments, called by javascript constructor 
	static TNodeJsFIn* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);
public:
	/**
	* Peeks a character
	* @returns {string} Character string.
	*/
	//# exports.FIn.prototype.peekCh= function() { return ''; }
	JsDeclareFunction(peekCh);	
	
	/**
	* Reads a character
	* @returns {string} Character string.
	*/
	//# exports.FIn.prototype.getCh= function() { return ''; }
	JsDeclareFunction(getCh);
	
	/**
	* Reads a line	
	* @returns {string} Line string.
	*/
	//# exports.FIn.prototype.readLine = function() { return ''; }
	JsDeclareFunction(readLine);
    
	/**
	* Reads a string that was serialized using `fs.FOut.writeBinary`.
	* @returns {string} String
	*/
    //# exports.FIn.prototype.readString = function() { return ''; }
    JsDeclareFunction(readString);
	
	/**
	* @property {boolean} eof - True if end of file is detected.
	*/
	//# exports.FIn.prototype.eof = false;
	JsDeclareProperty(eof);

	/**
	* @property {number} length - Length of input stream.
	*/
	//# exports.FIn.prototype.length = 0;
	JsDeclareProperty(length);

	/**
	* Reads the whole stream
	* @returns {string} Content of the file.
	*/
	//# exports.FIn.prototype.readAll = function() { return ''; }
	JsDeclareFunction(readAll);

	/**
	* Closes the input stream.
	*/
	//# exports.FIn.prototype.close = function() { return ''; }
	JsDeclareFunction(close);

	/**
	* Checks if the input stream is closed.
	*/
	//# exports.FIn.prototype.isClosed = function() { return ''; }
	JsDeclareFunction(isClosed);
};


///////////////////////////////
// NodeJs-FOut
class TNodeJsFOut : public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	static v8::Persistent<v8::Function> Constructor;
public:
	static void Init(v8::Handle<v8::Object> exports);
	static const TStr GetClassId() { return "FOut"; }

	// wrapped C++ object
	PSOut SOut;
	// C++ constructor
    TNodeJsFOut(const TStr& FilePath, const bool& AppendP):
        SOut(TFOut::New(FilePath, AppendP)) { }
    TNodeJsFOut(const TStr& FilePath): SOut(TZipOut::NewIfZip(FilePath)) { }
	TNodeJsFOut(const PSOut& _SOut) : SOut(_SOut) { }

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
	//# exports.FOut = function(fileName, append) {}	
	static TNodeJsFOut* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);
public:
	/**
	* Writes a string or number or a JSON object in human readable form
	* @param {(String | Number | Object)} arg - Argument to write
	* @returns {module:fs.FOut} Self.
	*/
	//# exports.FOut.prototype.write = function(arg) { return this; }
	JsDeclareFunction(write);

	/**
	* Writes a string or number or a JSON object in binary form
	* @param {(String | Number | Object)} str - Argument to write
	* @returns {module:fs.FOut} Self.
	*/
	//# exports.FOut.prototype.writeBinary = function(arg) { return this; }
	JsDeclareFunction(writeBinary);

	/**
	* Writes a string and adds a new line
	* @param {String} str - String to write
	* @returns {module:fs.FOut} Self.
	*/
	//# exports.FOut.prototype.writeLine = function(str) { return this; }
    JsDeclareFunction(writeLine);
    
	/**
	* Flushes the output stream
	* @returns {module:fs.FOut} Self.
	*/
	//# exports.FOut.prototype.flush = function() { return this; }
    JsDeclareFunction(flush);

	/**
	* Closes the output stream
	*/
	//# exports.FOut.prototype.close = function() {}
    JsDeclareFunction(close);
};

#endif


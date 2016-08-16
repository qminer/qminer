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
        AllowedFPathV(AllowedDirV_) {}
    ~TNodeJsFs() {}

public:
    static void Init(v8::Handle<v8::Object> exports);
    
private:
    class TReadLinesCallback: public TMainThreadTask {
	private:
		v8::Persistent<v8::Function>* OnLine;
		PExcept Except;
	public:
		TVec<TStrV> CsvLineV;

		TReadLinesCallback(const int& BatchSize, v8::Persistent<v8::Function>* _OnLine):
			OnLine(_OnLine),
			Except(),
			CsvLineV(BatchSize, 0) {}
		void Run();
		PExcept GetExcept() const { return Except; }
	};

	class TReadCsvTask: public TNodeTask {
	private:
		PSIn SIn;
		int Offset;
		int Limit;
		int BatchSize;
		v8::Persistent<v8::Function> OnLine;
		TReadLinesCallback* LinesCallback;
		TMainThreadHandle* LinesHandle;

	public:
		TReadCsvTask(const v8::FunctionCallbackInfo<v8::Value>& Args);
		~TReadCsvTask();

		v8::Handle<v8::Function> GetCallback(const v8::FunctionCallbackInfo<v8::Value>& Args);
		void Run();

	private:
		void CallCallback();
	};

public:

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
	//# exports.openRead = function(fileName) { return Object.create(require('qminer').fs.FIn.prototype); }
    JsDeclareFunction(openRead);
    
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
	//# exports.openWrite = function(fileName) { return Object.create(require('qminer').fs.FOut.prototype); }
    JsDeclareFunction(openWrite);
	
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
	//# exports.openAppend = function(fileName) { return Object.create(require('qminer').fs.FOut.prototype); }	
	JsDeclareFunction(openAppend);
	
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
	//# exports.exists = function(fileName) { return false; }	
    JsDeclareFunction(exists);
	
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
    * var destination = fs.copy('text.txt', 'copy.txt');
	*/
	//# exports.copy = function(source, dest) { return ""; }	
    JsDeclareFunction(copy);
	
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
    * var destination = fs.move('text.txt', 'move.txt');
	*/
	//# exports.move = function(source, dest) { return ""; }
	JsDeclareFunction(move);
	
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
	//# exports.del = function(fileName) { return false; }	
    JsDeclareFunction(del);
	
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
	//# exports.rename = function(source, dest) { return ""; }
    JsDeclareFunction(rename);

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
	//# exports.fileInfo = function(fileName) { return { createTime : "",  lastAccessTime: "", lastWriteTime: "", size: 0 }}	
    JsDeclareFunction(fileInfo);
	
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
	//# exports.mkdir = function(dirName) { return false; }	
    JsDeclareFunction(mkdir);
	
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
	//# exports.rmdir = function(dirName) { return false; }
    JsDeclareFunction(rmdir);
	
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
	//# exports.listFile = function(dirName, fileExtension, recursive) { return ['']; }
    JsDeclareFunction(listFile);

    /**
     * Reads a buffer line by line and calls a callback for each line.
     * @param {String | module:fs.FIn | Buffer} buffer - Name of the file, input stream of a Node.js buffer.
     * @param {function} onLine - A callback that gets called on each line (for example: `function (line) {}`).
     * @param {function} onEnd - A callback that gets returned after all the lines have been read.
     * @param {function} onError - A callback that gets called if an error occurs.
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
     * //var numberOfLines = 0;
     * //function onLine(line) {
     * //    console.log(line);
     * //    numberOfLines += 1;
     * //}
     * //function onEnd(line) {
     * //    console.log("Number of lines", numberOfLines);
     * //}
     * //function onError(err) {
     * //    console.log(err);
     * //}
     * //fs.readLines(fin, onLine, onEnd, onError);
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
    ~TNodeJsFIn() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; }
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
	//# exports.FIn = function(fileName) { return Object.create(require('qminer').fs.FIn.prototype); }	
	// parses arguments, called by javascript constructor 
	static TNodeJsFIn* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);
public:
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
	//# exports.FIn.prototype.peekCh= function() { return ''; }
	JsDeclareFunction(peekCh);	
	
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
	//# exports.FIn.prototype.getCh= function() { return ''; }
	JsDeclareFunction(getCh);
	
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
	//# exports.FIn.prototype.readLine = function() { return ''; }
	JsDeclareFunction(readLine);
    
	/**
	* Reads a string that was serialized using `fs.FOut.writeBinary`.
	* @returns {string} String.
    * @example
    * // import fs module
    * var fs = require('qminer').fs;
    * // read a string that was serialized using fs.FOut.writeBinary
	*/
    //# exports.FIn.prototype.readString = function() { return ''; }
    JsDeclareFunction(readString);
	
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
	//# exports.FIn.prototype.eof = false;
	JsDeclareProperty(eof);

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
	//# exports.FIn.prototype.length = 0;
	JsDeclareProperty(length);

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
	//# exports.FIn.prototype.readAll = function() { return ''; }
	JsDeclareFunction(readAll);

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
	//# exports.FIn.prototype.close = function() { }
	JsDeclareFunction(close);

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
	//# exports.FIn.prototype.isClosed = function() { return false; }
	JsDeclareFunction(isClosed);
};


///////////////////////////////
// NodeJs-FOut
class TNodeJsFOut : public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	static v8::Persistent<v8::Function> Constructor;
    ~TNodeJsFOut() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; }
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
	//# exports.FOut = function(fileName, append) {}	
	static TNodeJsFOut* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);
public:
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
	//# exports.FOut.prototype.write = function(arg) { return this; }
	JsDeclareFunction(write);

	/**
	* Writes a string or number or a JSON object in binary form.
	* @param {(String | Number | Object)} str - Argument to write.
	* @returns {module:fs.FOut} Self.
    * @example
    * // import fs module
    * var fs = require('qminer').fs;
    * // save a string in binary form
	*/
	//# exports.FOut.prototype.writeBinary = function(arg) { return this; }
	JsDeclareFunction(writeBinary);

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
	//# exports.FOut.prototype.writeLine = function(str) { return this; }
    JsDeclareFunction(writeLine);
    
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
	//# exports.FOut.prototype.flush = function() { return this; }
    JsDeclareFunction(flush);

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
	//# exports.FOut.prototype.close = function() {}
    JsDeclareFunction(close);
};

#endif


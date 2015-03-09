#ifndef QMINER_FS_NODEJS_H
#define QMINER_FS_NODEJS_H


#include <node.h>
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
//!
//! ### File system
//! 

/**
* File-system module.
* @module fs
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
    /**
	* open file in read mode and return file input stream
	* @param {string} fileName - File name.
	* @returns {module:fs.FIn} fin - Input stream.
	*/
	//# exports.openRead = function(fileName) {}
    JsDeclareFunction(openRead);
    //!- `fout = fs.openWrite(fileName)` -- open file in write mode and return file output stream `fout`
    JsDeclareFunction(openWrite);
    //!- `fout = fs.openAppend(fileName)` -- open file in append mode and return file output stream `fout`
    JsDeclareFunction(openAppend);
    //!- `bool = fs.exists(fileName)` -- does file exist?
    JsDeclareFunction(exists);
    //!- `fs.copy(fromFileName, toFileName)` -- copy file
    JsDeclareFunction(copy);
    //!- `fs.move(fromFileName, toFileName)` -- move file
    JsDeclareFunction(move);
    //!- `fs.del(fileName)` -- delete file
    JsDeclareFunction(del);
    //!- `fs.rename(fromFileName, toFileName)` -- rename file
    JsDeclareFunction(rename);
    //!- `fileInfoJson = fs.fileInfo(fileName)` -- returns file info as a json object {createTime:str, lastAccessTime:str, lastWriteTime:str, size:num}.
    JsDeclareFunction(fileInfo);
    //!- `fs.mkdir(dirName)` -- make folder
    JsDeclareFunction(mkdir);
    //!- `fs.rmdir(dirName)` -- delete folder
    JsDeclareFunction(rmdir);
    //!- `strArr = fs.listFile(dirName)` -- returns list of files in directory
    //!- `strArr = fs.listFile(dirName, fileExtension)` -- returns list of files in directory given file extension
    //!- `strArr = fs.listFile(dirName, fileExtension, recursive)` -- returns list of files in directory given extension. `recursive` is a boolean
    JsDeclareFunction(listFile);
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
//    -define ClassId member
//    -set Constructor callback to TNodeJsUtil::_NewCpp (doesn't create a new pointer to the wrapper) and inherit from template
// creating from JS using 'new' 
//	  -implement NewFromArgs (parses arguments and returns pointer to wrapper)
//	  -define ClassId member
//    -set template callback to TNodeJsUtil::_NewJs (this creates a new pointer to the wrapper)
//    -attach template function to exports in Init function
private:
	static v8::Persistent<v8::Function> Constructor;
public:
	static void Init(v8::Handle<v8::Object> Exports);
	static const TStr ClassId;
	// wrapped C++ object
	PSIn SIn;
	// C++ constructor
	TNodeJsFIn(const TStr& FNm) : SIn(TZipIn::NewIfZip(FNm)) { }
private:	
	/**
	* Input file stream.
	* @classdesc Used for reading files.
	* @class
	* @param {String} fileName - File name
	*/
	//# exports.FIn = function(fnm) {}	
	// parses arguments, called by javascript constructor 
	static TNodeJsFIn* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);
public:
	//! 
	//! **Functions and properties:**
	//!   
	//!- `char = fin.peekCh()` -- peeks a character
	JsDeclareFunction(peekCh);
	//!- `char = fin.getCh()` -- reads a character
	JsDeclareFunction(getCh);
	//!- `line = fin.readLine()` -- reads a line
	/**
	* Reads a line	
	* @returns {String} line - Returns a line.
	*/
	//# exports.FIn.prototype.readLine = function() {}
	JsDeclareFunction(readLine);
	//!- `bool = fin.eof` -- end of stream?
	JsDeclareProperty(eof);
	//!- `len = fin.length` -- returns the length of input stream
	JsDeclareProperty(length);
	//!- `str = fin.readAll()` -- reads the whole file
	JsDeclareFunction(readAll);

};


///////////////////////////////
// NodeJs-FOut
//!
//! ### Output File Stream
//! 
class TNodeJsFOut : public node::ObjectWrap {
	friend class TNodeJsUtil;
public:
    PSOut SOut;
private:
    TNodeJsFOut(const TStr& FilePath, const bool& AppendP):
        SOut(TFOut::New(FilePath, AppendP)) { }
    TNodeJsFOut(const TStr& FilePath): SOut(TZipOut::NewIfZip(FilePath)) { }
public:
    static void Init(v8::Handle<v8::Object> exports);
	static TStr ClassId;
    static v8::Local<v8::Object> New(const TStr& FilePath, const bool& AppendP = false);

    //! 
    //! **Functions and properties:**
    //!     
	//!- `fout = new fs.FOut(fnm, append)` -- creates a new file output stream.
	/**
	* Output file stream.
	* @classdesc Used for writing files.
	* @class
	* @param {String} fileName - File name
	* @param {boolean} [append=false] - Append flag
	*/
	//# exports.FOut = function(fileName, append) {}	
	JsDeclareFunction(New);
    //!- `fout = fout.write(data)` -- writes to output stream. `data` can be a number, a json object or a string.
    /**
	* Writes a string
	* @param {String} str - String to write
	* @returns {module:fs.FOut} this - Returns self.
	*/
	//# exports.FOut.prototype.write = function(str) {}
	JsDeclareFunction(write);
    //!- `fout = fout.writeLine(data)` -- writes data to output stream and adds newline
    JsDeclareFunction(writeLine);
    //!- `fout = fout.flush()` -- flushes output stream
    JsDeclareFunction(flush);
    //!- `fout = fout.close()` -- closes output stream
    JsDeclareFunction(close);
private:
    static v8::Persistent<v8::Function> constructor;
};

#endif


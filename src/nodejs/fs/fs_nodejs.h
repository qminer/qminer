#ifndef QMINER_FS_NODEJS
#define QMINER_FS_NODEJS

#define BUILDING_NODE_EXTENSION

#include <node.h>
#include <node_object_wrap.h>
#include "base.h"
#include "util.h"

///////////////////////////////
/// JavaScript Directory
class TJsFPath {
private:
	/// Canonicalized file path 
	TStr CanonicalFPath; 
    
public:
	TJsFPath(const TStr& FPath = "./");
	
	/// Is directory subdirectory of this 
	bool IsSubdir(const TJsFPath& JsFPath) const; 
	/// Directory equals this 
	bool Equals(const TJsFPath& JsFPath) const; 
	/// Get directory name
	const TStr& GetFPath() const { return CanonicalFPath; }

	/// Load list of directories
	static void GetFPathV(const TStrV& FPathV, TVec<TJsFPath>& JsFPathV);
   /// Canonicalize file path 
   static TStr GetCanonicalPath(const TStr& FPath);
};

///////////////////////////////
// NodeJs-Filesystem
//#
//# ### File system
//# 
class TNodeJsFs : public node::ObjectWrap {
public:
	// directories we're allowed to access 
	TVec<TJsFPath> AllowedFPathV;
private:
   
	TNodeJsFs(const TVec<TJsFPath>& AllowedDirV_ = TVec<TJsFPath>()): AllowedFPathV(AllowedDirV_) { }
public:
	static void Init(v8::Handle<v8::Object> exports);
	//# 
	//# **Functions and properties:**
	//# 
    //#- `fin = fs.openRead(fileName)` -- open file in read mode and return file input stream `fin`
	JsDeclareFunction(openRead);
    //#- `fout = fs.openWrite(fileName)` -- open file in write mode and return file output stream `fout`
	JsDeclareFunction(openWrite);
    //#- `fout = fs.openAppend(fileName)` -- open file in append mode and return file output stream `fout`
	JsDeclareFunction(openAppend);
    //#- `bool = fs.exists(fileName)` -- does file exist?
	JsDeclareFunction(exists);
    //#- `fs.copy(fromFileName, toFileName)` -- copy file
	JsDeclareFunction(copy);
    //#- `fs.move(fromFileName, toFileName)` -- move file
	JsDeclareFunction(move);
    //#- `fs.del(fileName)` -- delete file
	JsDeclareFunction(del);
    //#- `fs.rename(fromFileName, toFileName)` -- rename file
	JsDeclareFunction(rename);
    //#- `fileInfoJson = fs.fileInfo(fileName)` -- returns file info as a json object {createTime:str, lastAccessTime:str, lastWriteTime:str, size:num}.
	JsDeclareFunction(fileInfo);
    //#- `fs.mkdir(dirName)` -- make folder
	JsDeclareFunction(mkdir);
    //#- `fs.rmdir(dirName)` -- delete folder
	JsDeclareFunction(rmdir);
    //#- `strArr = fs.listFile(dirName, fileExtension)` -- returns list of files in directory given file extension
    //#- `strArr = fs.listFile(dirName, fileExtension, recursive)` -- returns list of files in directory given extension. `recursive` is a boolean
	JsDeclareFunction(listFile);
};

///////////////////////////////
// NodeJs-FIn
//#
//# ### Input File Stream
//# 
class TNodeJsFIn : public node::ObjectWrap {
public:
   PSIn SIn;
private:
   // typedef TJsObjUtil<TJsFIn> TJsFInUtil;
   TNodeJsFIn(const TStr& FNm): SIn(TZipIn::NewIfZip(FNm)) { }
public:
	static void Init(v8::Handle<v8::Object> exports);
	static v8::Local<v8::Object> New(const TStr& FNm);
	/*
	static v8::Persistent<v8::Object> New(const TStr& FNm) {
		return TJsFInUtil::New(new TJsFIn(FNm)); }
   */
   static PSIn GetArgFIn(const v8::FunctionCallbackInfo<v8::Value>& Args,
      const int& ArgN);
   
    //# 
	 //# **Functions and properties:**
	 //#     
   JsDeclareFunction(New);
    //#- `char = fin.peekCh()` -- peeks a character
	JsDeclareFunction(peekCh);
    //#- `char = fin.getCh()` -- reads a character
	JsDeclareFunction(getCh);
    //#- `line = fin.readLine()` -- reads a line
	JsDeclareFunction(readLine);
    //#- `bool = fin.eof` -- end of stream?
	JsDeclareProperty(eof);
    //#- `len = fin.length` -- returns the length of input stream
	JsDeclareProperty(length);
	//#- `str = fin.readAll()` -- reads the whole file
	JsDeclareFunction(readAll);
private:
   static v8::Persistent<v8::Function> constructor;
};

///////////////////////////////
// NodeJs-FOut
//#
//# ### Output File Stream
//# 
class TNodeJsFOut : public node::ObjectWrap {
public:
	PSOut SOut;
private:
	TNodeJsFOut(const TStr& FilePath, const bool& AppendP): SOut(TFOut::New(FilePath, AppendP)) { }
	TNodeJsFOut(const TStr& FilePath): SOut(TZipOut::NewIfZip(FilePath)) { }
	TNodeJsFOut(PSOut& SOut_) : SOut(SOut_) { }
public:
	static void Init(v8::Handle<v8::Object> exports);
	static v8::Local<v8::Object> New(const TStr& FilePath, const bool& AppendP = false);
   static v8::Local<v8::Object> New(PSOut& SOut_);
   
   static PSOut GetArgFOut(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);
   
   //# 
	//# **Functions and properties:**
	//#     
   JsDeclareFunction(New);
    //#- `fout = fout.write(data)` -- writes to output stream. `data` can be a number, a json object or a string.
	JsDeclareFunction(write);
    //#- `fout = fout.writeLine(data)` -- writes data to output stream and adds newline
	JsDeclareFunction(writeLine);
    //#- `fout = fout.flush()` -- flushes output stream
	JsDeclareFunction(flush);
    //#- `fout = fout.close()` -- closes output stream
  	JsDeclareFunction(close);
private:
   static v8::Persistent<v8::Function> constructor;
};

#endif


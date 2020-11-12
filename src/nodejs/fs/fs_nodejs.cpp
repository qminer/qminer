/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include "fs_nodejs.h"

///////////////////////////////
// NodeJs-Directory
TNodeJsFPath::TNodeJsFPath(const TStr& FPath): CanonicalFPath(GetCanonicalPath(FPath)) { }

bool TNodeJsFPath::Equals(const TNodeJsFPath& JsFPath) const {
    return CanonicalFPath == JsFPath.GetFPath();
}

bool TNodeJsFPath::IsSubdir(const TNodeJsFPath& JsFPath) const {
    return CanonicalFPath.StartsWith(JsFPath.GetFPath());
}

void TNodeJsFPath::GetFPathV(const TStrV& FPathV, TVec<TNodeJsFPath>& JsFPathV) {
    for (TStrV::TIter It = FPathV.BegI(); It != FPathV.EndI(); ++It) {
        JsFPathV.Add(TNodeJsFPath(*It));
    }
}

TStr TNodeJsFPath::GetCanonicalPath(const TStr& FPath) {
    // Get absolute path
    TStr AbsFPath = TStr::GetNrAbsFPath(FPath);
    // Remove any redundancies
    TStrV CanonV; AbsFPath.SplitOnAllCh('/', CanonV);
    TSStack<TStr> CanonS; TStr CurrStr;
    for (int ElN = 0; ElN < CanonV.Len(); ++ElN) {
        CurrStr = CanonV.GetVal(ElN);
        if (CurrStr == "..") {
            EAssertR(!CanonS.Empty(), "Stack empty");
            CanonS.Pop();
        } else if (CurrStr != ".") {
            CanonS.Push(CurrStr+"/");
        }
    }
    // Assemble the canonical path (from left to right
    EAssertR(!CanonS.Empty(), "Stack empty");
    // We start with drive letter (Windows) or slash (Unix)
    TChA CanonFPath = AbsFPath.LeftOf('/'); CanonFPath += '/';
    // Get the rest of the path
    for (int CanonN = CanonS.Len() - 1; CanonN >= 0; CanonN--) {
        CanonFPath += CanonS[CanonN];
    }
    // Done
    return CanonFPath;
}

///////////////////////////////
// NodeJs-Filesystem
void TNodeJsFs::Init(v8::Local<v8::Object> exports) {
    // Add all prototype methods, getters and setters here.
    NODE_SET_METHOD(exports, "openRead", _openRead);
    NODE_SET_METHOD(exports, "openWrite", _openWrite);
    NODE_SET_METHOD(exports, "openAppend", _openAppend);
    NODE_SET_METHOD(exports, "exists", _exists);
    NODE_SET_METHOD(exports, "copy", _copy);
    NODE_SET_METHOD(exports, "move", _move);
    NODE_SET_METHOD(exports, "del", _del);
    NODE_SET_METHOD(exports, "rename", _rename);
    NODE_SET_METHOD(exports, "fileInfo", _fileInfo);
    NODE_SET_METHOD(exports, "mkdir", _mkdir);
    NODE_SET_METHOD(exports, "rmdir", _rmdir);
    NODE_SET_METHOD(exports, "listFile", _listFile);
    NODE_SET_METHOD(exports, "readLines", _readLines);
}

void TNodeJsFs::openRead(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1 && Args[0]->IsString(), "Expected file path.");
    TStr FNm(*Nan::Utf8String (TNodeJsUtil::ToLocal(Nan::To<v8::String>(Args[0]))));
    // file exist check is done by TFIn

    Args.GetReturnValue().Set(
        TNodeJsUtil::NewInstance<TNodeJsFIn>(new TNodeJsFIn(FNm)));
}

void TNodeJsFs::openWrite(const v8::FunctionCallbackInfo<v8::Value>& Args) { // Call withb AppendP = false
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1 && Args[0]->IsString(), "Expected file path.");
    TStr FNm(*Nan::Utf8String (TNodeJsUtil::ToLocal(Nan::To<v8::String>(Args[0]))));

    Args.GetReturnValue().Set(
        TNodeJsUtil::NewInstance<TNodeJsFOut>(new TNodeJsFOut(FNm, false)));
}

void TNodeJsFs::openAppend(const v8::FunctionCallbackInfo<v8::Value>& Args) { // Call with AppendP = true
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1 && Args[0]->IsString(), "Expected file path.");
    TStr FNm(*Nan::Utf8String (TNodeJsUtil::ToLocal(Nan::To<v8::String>(Args[0]))));

    Args.GetReturnValue().Set(
        TNodeJsUtil::NewInstance<TNodeJsFOut>(new TNodeJsFOut(FNm, true)));
}

void TNodeJsFs::exists(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1 && Args[0]->IsString(), "Expected file path.");
    TStr FNm(*Nan::Utf8String (TNodeJsUtil::ToLocal(Nan::To<v8::String>(Args[0]))));
    Args.GetReturnValue().Set(Nan::New(TFile::Exists(FNm)));
}

void TNodeJsFs::copy(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 2 && Args[0]->IsString() && Args[1]->IsString(),
        "Expected 2 arguments: source and destination file paths.");
    TStr SrcFNm(*Nan::Utf8String (TNodeJsUtil::ToLocal(Nan::To<v8::String>(Args[0]))));
    EAssertR(TFile::Exists(SrcFNm), "File '" + SrcFNm + "' does not exist");
    TStr DstFNm(*Nan::Utf8String (TNodeJsUtil::ToLocal(Nan::To<v8::String>(Args[1]))));
    TFile::Copy(SrcFNm, DstFNm);
    Args.GetReturnValue().Set(Nan::Undefined());
}

void TNodeJsFs::move(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 2 && Args[0]->IsString() && Args[1]->IsString(),
        "Expected 2 arguments: source and destination file paths.");
    TStr SrcFNm(*Nan::Utf8String (TNodeJsUtil::ToLocal(Nan::To<v8::String>(Args[0]))));
    EAssertR(TFile::Exists(SrcFNm), TStr("File '" + SrcFNm + "' does not exist").CStr());
    TStr DstFNm(*Nan::Utf8String (TNodeJsUtil::ToLocal(Nan::To<v8::String>(Args[1]))));
    TFile::Copy(SrcFNm, DstFNm);
    TFile::Del(SrcFNm, false); // ThrowExceptP = false
    Args.GetReturnValue().Set(Nan::Undefined());
}

void TNodeJsFs::del(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1 && Args[0]->IsString(),
        "Expected a file path as the only argument.");
    TStr FNm(*Nan::Utf8String (TNodeJsUtil::ToLocal(Nan::To<v8::String>(Args[0]))));
    EAssertR(TFile::Exists(FNm), TStr("File '" + FNm + "' does not exist").CStr());
    Args.GetReturnValue().Set(Nan::New(TFile::Del(FNm, false))); // ThrowExceptP = false
}

void TNodeJsFs::rename(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 2 && Args[0]->IsString() && Args[1]->IsString(),
        "Expected 2 arguments: source and destination file paths.");
    TStr SrcFNm(*Nan::Utf8String (TNodeJsUtil::ToLocal(Nan::To<v8::String>(Args[0]))));
    EAssertR(TFile::Exists(SrcFNm), TStr("File '" + SrcFNm + "' does not exist").CStr());
    TStr DstFNm(*Nan::Utf8String (TNodeJsUtil::ToLocal(Nan::To<v8::String>(Args[1]))));
    TFile::Rename(SrcFNm, DstFNm);
    Args.GetReturnValue().Set(Nan::Undefined());
}

void TNodeJsFs::fileInfo(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1 && Args[0]->IsString(),
        "Expected a file path as the only argument.");
    TStr FNm(*Nan::Utf8String (TNodeJsUtil::ToLocal(Nan::To<v8::String>(Args[0]))));
    EAssertR(TFile::Exists(FNm), TStr("File '" + FNm + "' does not exist").CStr());
    const uint64 CreateTm = TFile::GetCreateTm(FNm);
    const uint64 LastAccessTm = TFile::GetLastAccessTm(FNm);
    const uint64 LastWriteTm = TFile::GetLastWriteTm(FNm);
    const uint64 Size = TFile::GetSize(FNm);
    v8::Local<v8::Object> Obj = v8::Object::New(Isolate);
    Nan::Set(Obj, TNodeJsUtil::ToLocal(Nan::New("createTime")),
        TNodeJsUtil::ToLocal(Nan::New(TTm::GetTmFromMSecs(CreateTm).GetWebLogDateTimeStr().CStr())));
    Nan::Set(Obj, TNodeJsUtil::ToLocal(Nan::New("lastAccessTime")),
        TNodeJsUtil::ToLocal(Nan::New(TTm::GetTmFromMSecs(LastAccessTm).GetWebLogDateTimeStr().CStr())));
    Nan::Set(Obj, TNodeJsUtil::ToLocal(Nan::New("lastWriteTime")),
        TNodeJsUtil::ToLocal(Nan::New(TTm::GetTmFromMSecs(LastWriteTm).GetWebLogDateTimeStr().CStr())));
    Nan::Set(Obj, TNodeJsUtil::ToLocal(Nan::New("size")),
        Nan::New(static_cast<double>(Size)));
    Args.GetReturnValue().Set(Obj);
}

void TNodeJsFs::mkdir(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1 && Args[0]->IsString(),
        "Expected directory name as the only argument.");
    TStr FPath(*Nan::Utf8String (TNodeJsUtil::ToLocal(Nan::To<v8::String>(Args[0]))));
    const bool GenDirP = TDir::GenDir(FPath);
    Args.GetReturnValue().Set(Nan::New(GenDirP));
}

void TNodeJsFs::rmdir(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1 && Args[0]->IsString(),
        "Expected directory name as the only argument.");
    TStr FPath(*Nan::Utf8String (TNodeJsUtil::ToLocal(Nan::To<v8::String>(Args[0]))));
    const bool DelDirP = TDir::DelDir(FPath);
    Args.GetReturnValue().Set(Nan::New(DelDirP));
}

void TNodeJsFs::listFile(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() >= 1 && Args[0]->IsString(),
        "Expected directory path as the first argument.");
    // Read parameters
    TStr FPath(*Nan::Utf8String (TNodeJsUtil::ToLocal(Nan::To<v8::String>(Args[0]))));
    TStrV FExtV;
    if (Args.Length() >= 2 && Args[1]->IsString()) {
        FExtV.Add(TStr(*Nan::Utf8String (TNodeJsUtil::ToLocal(Nan::To<v8::String>(Args[1])))));
    }
    const bool RecurseP = Args.Length() >= 3 && Args[2]->IsBoolean() && Nan::To<bool>(Args[2]).FromJust();
    // Get file list
    TStrV FNmV;
    TFFile::GetFNmV(FPath, FExtV, RecurseP, FNmV);
    FNmV.Sort();
    v8::Local<v8::Array> FNmArr = v8::Array::New(Isolate, FNmV.Len());
    for(int FldN = 0; FldN < FNmV.Len(); ++FldN) {
        Nan::Set(FNmArr, v8::Integer::New(Isolate, FldN), TNodeJsUtil::ToLocal(Nan::New(FNmV.GetVal(FldN).CStr())));
    }
    Args.GetReturnValue().Set(FNmArr);
}

void TNodeJsFs::readLines(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 3, "TNodeJsFs::readLines: Invalid number of arguments!");
    EAssertR(!TNodeJsUtil::IsArgNull(Args, 0), "TNodeJsFs::readLines: Buffer is null or undefined!");

    PSIn SIn;
    if (TNodeJsUtil::IsArgStr(Args, 0)) {
        // Read from file
        const TStr FNm = TNodeJsUtil::GetArgStr(Args, 0);
        SIn = TFIn::New(FNm);
    } else if (TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsFIn::GetClassId())) {
        // Read from input stream
        TNodeJsFIn* JsFIn = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFIn>(Args, 0);
        SIn = JsFIn->SIn;
    } else {
        // Read from Node.js Buffer
        EAssertR(TNodeJsUtil::IsArgBuffer(Args, 0), "TNodeJsFs::readLines: argument not a buffer");
        v8::Local<v8::Object> BuffObj = TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0]));
        char* Buff = node::Buffer::Data(BuffObj);
        size_t BuffLen = node::Buffer::Length(BuffObj);
        SIn = new TThinMIn(Buff, (int)BuffLen);
    }

    v8::Local<v8::Function> LineCallback = TNodeJsUtil::GetArgFun(Args, 1);
    v8::Local<v8::Function> EndCallback = TNodeJsUtil::GetArgFun(Args, 2);

    TStr LineStr;
    while (SIn->GetNextLn(LineStr)) {
        bool ContinueLoop = true;
        v8::Local<v8::String> LineV8Str = TNodeJsUtil::ToLocal(Nan::New(LineStr.CStr()));
        ContinueLoop = TNodeJsUtil::ExecuteBool(LineCallback, LineV8Str);
        if (!ContinueLoop) { break; }
    }

    TNodeJsUtil::ExecuteVoid(EndCallback);
    Args.GetReturnValue().Set(Nan::Undefined());
}

///////////////////////////////
// NodeJs-FIn
v8::Persistent<v8::Function> TNodeJsFIn::Constructor;

void TNodeJsFIn::Init(v8::Local<v8::Object> exports) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::Local<v8::Context> context = Nan::GetCurrentContext();

    // template for creating function from javascript using "new", uses _NewJs callback
    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsFIn>);
    // child will have the same properties and methods, but a different callback: _NewCpp
    v8::Local<v8::FunctionTemplate> child = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewCpp<TNodeJsFIn>);
    child->Inherit(tpl);

    child->SetClassName(TNodeJsUtil::ToLocal(Nan::New(GetClassId().CStr())));
    // ObjectWrap uses the first internal field to store the wrapped pointer
    child->InstanceTemplate()->SetInternalFieldCount(1);

    tpl->SetClassName(TNodeJsUtil::ToLocal(Nan::New(GetClassId().CStr())));
    // ObjectWrap uses the first internal field to store the wrapped pointer
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Add all prototype methods, getters and setters here
    NODE_SET_PROTOTYPE_METHOD(tpl, "peekCh", _peekCh);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getCh", _getCh);
    NODE_SET_PROTOTYPE_METHOD(tpl, "readLine", _readLine);
    NODE_SET_PROTOTYPE_METHOD(tpl, "readString", _readString);
    NODE_SET_PROTOTYPE_METHOD(tpl, "readAll", _readAll);
    NODE_SET_PROTOTYPE_METHOD(tpl, "close", _close);
    NODE_SET_PROTOTYPE_METHOD(tpl, "isClosed", _isClosed);

    // Add properties
    tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("eof")), _eof);
    tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("length")), _length);

    // This has to be last, otherwise the properties won't show up on the object in JavaScript
    // Constructor is used when creating the object from C++
    Constructor.Reset(Isolate, TNodeJsUtil::ToLocal(child->GetFunction(context)));
    // we need to export the class for calling using "new FIn(...)"
    Nan::Set(exports, TNodeJsUtil::ToLocal(Nan::New(GetClassId().CStr())),
        TNodeJsUtil::ToLocal(tpl->GetFunction(context)));
}

TNodeJsFIn* TNodeJsFIn::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    // parse arguments
    EAssertR(Args.Length() == 1 && Args[0]->IsString(), "Expected a file path.");
    return new TNodeJsFIn(*Nan::Utf8String (TNodeJsUtil::ToLocal(Nan::To<v8::String>(Args[0]))));
}


void TNodeJsFIn::peekCh(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Args.This());

    Args.GetReturnValue().Set(TNodeJsUtil::ToLocal(Nan::New(TStr(JsFIn->SIn->PeekCh()).CStr())));
}

void TNodeJsFIn::getCh(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Args.This());

    Args.GetReturnValue().Set(TNodeJsUtil::ToLocal(Nan::New(TStr(JsFIn->SIn->GetCh()).CStr())));
}

void TNodeJsFIn::readLine(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Args.This());
    TChA LnChA; JsFIn->SIn->GetNextLnBf(LnChA);

    Args.GetReturnValue().Set(TNodeJsUtil::ToLocal(Nan::New(LnChA.CStr())));
}

void TNodeJsFIn::readString(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Args.This());
    EAssertR(!JsFIn->SIn.Empty(), "Input stream is closed!");
    TStr Str = TStr(*JsFIn->SIn);

    Args.GetReturnValue().Set(
        TNodeJsUtil::ToLocal(Nan::New(Str.CStr())));
}

void TNodeJsFIn::readAll(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Args.This());
    TStr Res = TStr::LoadTxt(JsFIn->SIn);

    Args.GetReturnValue().Set(TNodeJsUtil::ToLocal(Nan::New(Res.CStr())));
}

void TNodeJsFIn::close(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Args.This());
    if (!JsFIn->SIn.Empty()) {
        JsFIn->SIn.Clr();
    }

    Args.GetReturnValue().Set(Nan::Undefined());
}

void TNodeJsFIn::isClosed(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Args.This());

    Args.GetReturnValue().Set(Nan::New(JsFIn->SIn.Empty()));
}

void TNodeJsFIn::eof(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Info.Holder());

    Info.GetReturnValue().Set(Nan::New(JsFIn->SIn->Eof()));
}

void TNodeJsFIn::length(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Info.Holder());

    Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsFIn->SIn->Len()));
}

///////////////////////////////
// NodeJs-FOut
v8::Persistent<v8::Function> TNodeJsFOut::Constructor;

void TNodeJsFOut::Init(v8::Local<v8::Object> exports) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::Local<v8::Context> context = Nan::GetCurrentContext();

    // template for creating function from javascript using "new", uses _NewJs callback
    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsFOut>);
    // child will have the same properties and methods, but a different callback: _NewCpp
    v8::Local<v8::FunctionTemplate> child = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewCpp<TNodeJsFOut>);
    child->Inherit(tpl);

    child->SetClassName(TNodeJsUtil::ToLocal(Nan::New(GetClassId().CStr())));
    // ObjectWrap uses the first internal field to store the wrapped pointer
    child->InstanceTemplate()->SetInternalFieldCount(1);

    tpl->SetClassName(TNodeJsUtil::ToLocal(Nan::New(GetClassId().CStr())));
    // ObjectWrap uses the first internal field to store the wrapped pointer
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Add all prototype methods, getters and setters here.
    NODE_SET_PROTOTYPE_METHOD(tpl, "write", _write);
    NODE_SET_PROTOTYPE_METHOD(tpl, "writeBinary", _writeBinary);
    NODE_SET_PROTOTYPE_METHOD(tpl, "writeLine", _writeLine);
    NODE_SET_PROTOTYPE_METHOD(tpl, "flush", _flush);
    NODE_SET_PROTOTYPE_METHOD(tpl, "close", _close);

    // This has to be last, otherwise the properties won't show up on the object in JavaScript
    // Constructor is used when creating the object from C++
    Constructor.Reset(Isolate, TNodeJsUtil::ToLocal(child->GetFunction(context)));
    // we need to export the class for calling using "new FIn(...)"
    Nan::Set(exports, TNodeJsUtil::ToLocal(Nan::New(GetClassId().CStr())),
        TNodeJsUtil::ToLocal(tpl->GetFunction(context)));
}

TNodeJsFOut* TNodeJsFOut::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    // parse arguments
    EAssertR(Args.Length() >= 1 && Args[0]->IsString(),
        "Expected file path.");
    TStr FNm(*Nan::Utf8String (TNodeJsUtil::ToLocal(Nan::To<v8::String>(Args[0]))));
    bool AppendP = Args.Length() >= 2 && Args[1]->IsBoolean() && Nan::To<bool>(Args[1]).FromJust();

    return new TNodeJsFOut(FNm, AppendP);
}

void TNodeJsFOut::write(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    EAssertR(Args.Length() == 1, "Invalid number of arguments to fout.write()");
    TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args.This());
    EAssertR(!JsFOut->SOut.Empty(), "Output stream already closed!");
    if (Args[0]->IsString()) {
        JsFOut->SOut->PutStr(*Nan::Utf8String (TNodeJsUtil::ToLocal(Nan::To<v8::String>(Args[0]))));
    } else if (Args[0]->IsInt32()) {
        JsFOut->SOut->PutStr(TInt::GetStr(Nan::To<int32_t>(Args[0]).FromJust()));
    } else if (Args[0]->IsNumber()) {
        JsFOut->SOut->PutStr(TFlt::GetStr(Nan::To<double>(Args[0]).FromJust()));
    } else if (TNodeJsUtil::IsArgJson(Args, 0)) {
        JsFOut->SOut->PutStr(TJsonVal::GetStrFromVal(TNodeJsUtil::GetArgJson(Args, 0)));
    } else {
        EFailR("Invalid type passed to fout.write() function.");
    }

    Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsFOut::writeBinary(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    EAssertR(Args.Length() == 1, "Invalid number of arguments to fout.write()");
    TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args.This());
    EAssertR(!JsFOut->SOut.Empty(), "Output stream already closed!");
    if (Args[0]->IsString()) {
        TStr Str = TNodeJsUtil::GetArgStr(Args, 0);
        Str.Save(*JsFOut->SOut);
    } else if (Args[0]->IsNumber()) {
        JsFOut->SOut->Save(Nan::To<double>(Args[0]).FromJust());
    } else if (TNodeJsUtil::IsArgJson(Args, 0)) {
        PJsonVal JsonVal = TNodeJsUtil::GetArgJson(Args, 0);
        JsonVal->Save(*JsFOut->SOut);
    } else {
        EFailR("Invalid type passed to fout.write() function.");
    }

    Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsFOut::writeLine(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    write(Args);

    TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args.This());
    EAssertR(!JsFOut->SOut.Empty(), "Output stream already closed!");
    JsFOut->SOut->PutLn();

    Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsFOut::flush(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args.This());
    EAssertR(!JsFOut->SOut.Empty(), "Output stream already closed!");
    JsFOut->SOut->Flush();
    Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsFOut::close(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args.This());
    EAssertR(!JsFOut->SOut.Empty(), "Output stream already closed!");

    JsFOut->SOut->Flush();
    JsFOut->SOut.Clr();

    Args.GetReturnValue().Set(Args.Holder());
}

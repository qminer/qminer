/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef QMINER_HT_NODEJS_H
#define QMINER_HT_NODEJS_H

#include <node.h>
#include <node_object_wrap.h>
#include "base.h"
#include "../nodeutil.h"
#include "../fs/fs_nodejs.h"

///////////////////////////////
// NodeJs-Hash-Map

class TAuxStrIntH {
public:
    static const TStr ClassId; //ClassId is set to "TStrIntH"
    static TStr GetArgKey(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
        // TJsBase is arbitrary here
        return TNodeJsUtil::GetArgStr(Args, ArgN);
    }
    static v8::Local<v8::Value> WrapKey(const TStr& Val) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope EscapableHandleScope(Isolate);
        v8::MaybeLocal<v8::String> TmpString = TNodeJsUtil::ToLocal(Nan::New(Val.CStr()));
        return EscapableHandleScope.Escape(TNodeJsUtil::ToLocal(TmpString));
    }
    static TInt GetArgDat(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
        // TJsBase is arbitrary here
        return TNodeJsUtil::GetArgInt32(Args, ArgN);
    }
    static v8::Local<v8::Value> WrapDat(const int& Val) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope EscapableHandleScope(Isolate);
        return EscapableHandleScope.Escape(v8::Int32::New(Isolate, Val));
    }
};

class TAuxStrFltH {
public:
    static const TStr ClassId; //ClassId is set to "TStrFltH"
    static TStr GetArgKey(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
        // TJsBase is arbitrary here
        return TNodeJsUtil::GetArgStr(Args, ArgN);
    }
    static v8::Local<v8::Value> WrapKey(const TStr& Val) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope EscapableHandleScope(Isolate);
        v8::MaybeLocal<v8::String> TmpString = TNodeJsUtil::ToLocal(Nan::New(Val.CStr()));
        return EscapableHandleScope.Escape(TNodeJsUtil::ToLocal(TmpString));
    }
    static TFlt GetArgDat(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
        // TJsBase is arbitrary here
        return TNodeJsUtil::GetArgFlt(Args, ArgN);
    }
    static v8::Local<v8::Value> WrapDat(const double& Val) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope EscapableHandleScope(Isolate);
        return EscapableHandleScope.Escape(Nan::New(Val));
    }
};

class TAuxStrStrH {
public:
    static const TStr ClassId; //ClassId is set to "TStrStrH"
    static TStr GetArgKey(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
        // TJsBase is arbitrary here
        return TNodeJsUtil::GetArgStr(Args, ArgN);
    }
    static v8::Local<v8::Value> WrapKey(const TStr& Val) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope EscapableHandleScope(Isolate);
        v8::MaybeLocal<v8::String> TmpString = TNodeJsUtil::ToLocal(Nan::New(Val.CStr()));
        return EscapableHandleScope.Escape(TNodeJsUtil::ToLocal(TmpString));
    }
    static TStr GetArgDat(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
        // TJsBase is arbitrary here
        return TNodeJsUtil::GetArgStr(Args, ArgN);
    }
    static v8::Local<v8::Value> WrapDat(const TStr& Val) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope EscapableHandleScope(Isolate);
        v8::MaybeLocal<v8::String> TmpString = TNodeJsUtil::ToLocal(Nan::New(Val.CStr()));
        return EscapableHandleScope.Escape(TNodeJsUtil::ToLocal(TmpString));
    }
};

class TAuxIntIntH {
public:
    static const TStr ClassId; //ClassId is set to "TIntIntH"
    static TInt GetArgKey(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
        // TJsBase is arbitrary here
        return TNodeJsUtil::GetArgInt32(Args, ArgN);
    }
    static v8::Local<v8::Value> WrapKey(const TInt& Val) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope EscapableHandleScope(Isolate);
        return EscapableHandleScope.Escape(v8::Int32::New(Isolate, Val));
    }
    static TInt GetArgDat(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
        // TJsBase is arbitrary here
        return TNodeJsUtil::GetArgInt32(Args, ArgN);
    }
    static v8::Local<v8::Value> WrapDat(const int& Val) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope EscapableHandleScope(Isolate);
        return EscapableHandleScope.Escape(v8::Int32::New(Isolate, Val));
    }
};

class TAuxIntFltH {
public:
    static const TStr ClassId; //ClassId is set to "TIntFltH"
    static TInt GetArgKey(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
        // TJsBase is arbitrary here
        return TNodeJsUtil::GetArgInt32(Args, ArgN);
    }
    static v8::Local<v8::Value> WrapKey(const TInt& Val) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope EscapableHandleScope(Isolate);
        return EscapableHandleScope.Escape(v8::Int32::New(Isolate, Val));
    }
    static TFlt GetArgDat(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
        // TJsBase is arbitrary here
        return TNodeJsUtil::GetArgFlt(Args, ArgN);
    }
    static v8::Local<v8::Value> WrapDat(const double& Val) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope EscapableHandleScope(Isolate);
        return EscapableHandleScope.Escape(Nan::New(Val));
    }
};

class TAuxIntStrH {
public:
    static const TStr ClassId; //ClassId is set to "TIntStrH"
    static TInt GetArgKey(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
        // TJsBase is arbitrary here
        return TNodeJsUtil::GetArgInt32(Args, ArgN);
    }
    static v8::Local<v8::Value> WrapKey(const TInt& Val) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope EscapableHandleScope(Isolate);
        return EscapableHandleScope.Escape(v8::Int32::New(Isolate, Val));
    }
    static TStr GetArgDat(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
        // TJsBase is arbitrary here
        return TNodeJsUtil::GetArgStr(Args, ArgN);
    }
    static v8::Local<v8::Value> WrapDat(const TStr& Val) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope EscapableHandleScope(Isolate);
        v8::MaybeLocal<v8::String> TmpString = TNodeJsUtil::ToLocal(Nan::New(Val.CStr()));
        return EscapableHandleScope.Escape(TNodeJsUtil::ToLocal(TmpString));
    }
};


template <class TKey = TStr, class TDat = TInt, class TAux = TAuxStrIntH>
class TNodeJsHash : public node::ObjectWrap {
	friend class TNodeJsUtil;
public:

    typedef THash<TKey, TDat> TKeyDatH;
    TKeyDatH Map;

	TNodeJsHash() : Map() {}
	TNodeJsHash(TSIn& SIn) : Map() { Map.Load(SIn); }

	static v8::Local<v8::Object> WrapInst(v8::Local<v8::Object> Obj);
	static v8::Local<v8::Object> WrapInst(v8::Local<v8::Object> Obj, TSIn& SIn);

    static void Init(v8::Local<v8::Object> exports);

    static TKeyDatH& GetMap(const v8::Local<v8::Object> Obj) {
        TNodeJsHash<TKey, TDat, TAux>* JsMap = ObjectWrap::Unwrap<TNodeJsHash<TKey, TDat, TAux> >(Obj);
        return JsMap->Map;
    }
    static void SetMap(const v8::Local<v8::Object> Obj, const TKeyDatH& _Map) {
        TNodeJsHash<TKey, TDat, TAux>* JsMap = ObjectWrap::Unwrap<TNodeJsHash<TKey, TDat, TAux> >(Obj);
        JsMap->Map = _Map;
    }

	/**
	* <% title %>
	* @classdesc Used for storing key/data pairs, wraps an efficient C++ implementation.
	* @class
	* @example
	* // create a new hashtable
	* ht = require('qminer').ht;
	* var h = new ht.<% className %>();
	* // Adding two key/dat pairs
	* h.put(<% key1 %>, <% val1 %>);
	* h.put(<% key2 %>, <% val2 %>);
	* // Getting data
	* h.hasKey(<% key1 %>); // returns true
	* h.get(<% key2 %>); // returns <% val2 %>
	* h.key(1); // returns <% key2 %>
	* h.dat(1); // returns <% val2 %>
	* h.length; // returns 2
	* // Saving and loading:
	* var fs = require('qminer').fs;
	* fout = fs.openWrite('map.dat'); // open write stream
	* h.save(fout).close(); // save and close write stream
	* var h2 = new ht.<% className %>(); // new empty table
	* var fin = fs.openRead('map.dat'); // open read stream
	* h2.load(fin); // load
	*/
	//# exports.<% className %> = function() {}
	JsDeclareFunction(New);

	/**
	* Returns dat given key.
	* @param {<% keyType %>} key - Hashmap key.
	* @returns {<% datType %>} Hashmap data.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.<% className %>();
    * // add a key/dat pair
    * h.put(<% key1 %>, <% val1 %>);
    * // get the newly added data
    * var val = h.get(<% key1 %>); // returns <% val1 %>
	*/
    //# exports.<% className %>.prototype.get = function(key) { return <% defaultVal %>; }
    JsDeclareFunction(get);

	/**
	* Add/update key-value pair.
	* @param {<% keyType %>} key - Hashmap key.
	* @param {<% datType %>} data - Hashmap data.
	* @returns {module:ht.<% className %>} Self.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.<% className %>();
    * // add a key/dat pair
    * h.put(<% key1 %>, <% val1 %>);
	*/
    //# exports.<% className %>.prototype.put = function(key, data) { return this; }
    JsDeclareFunction(put);

	/**
	* Returns true if the map has a given key.
	* @param {<% keyType %>} key - Hashmap key.
	* @returns {boolean} True if the map contains key. Otherwise, false.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.<% className %>();
    * // add a key/dat pair
    * h.put(<% key1 %>, <% val1 %>);
    * // check if the hashtable has the key
    * h.hasKey(<% key1 %>); // returns true
	*/
    //# exports.<% className %>.prototype.hasKey = function(key) { return false; }
    JsDeclareFunction(hasKey);

	/**
    * Number of key/dat pairs. Type `number`.
    * @example
    * // create a new hashtable
	* ht = require('qminer').ht;
	* var h = new ht.<% className %>();
	* // Adding two key/dat pairs
	* h.put(<% key1 %>, <% val1 %>);
    * // get the number of key/dat pairs
    * var length = h.length; // returns 1
	*/
	//# exports.<% className %>.prototype.length = 0;
	JsDeclareProperty(length);

	/**
	* Returns n-th key.
	* @param {number} n - Hashmap key index number. Should be between 0 and length-1.
	* @returns {<% keyType %>} The n-th key.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.<% className %>();
    * // add a key/dat pair
    * h.put(<% key1 %>, <% val1 %>);
    * // get the first key
    * var key = h.key(0); // returns <% key1 %>
	*/
    //# exports.<% className %>.prototype.key = function(n) { return <% defaultKey %>; }
    JsDeclareFunction(key);

    /**
    * Returns the ID of the key provided as parameter.
    * @param {<% keyType %>} key - Hashmap key.
    * @returns {number} n - Hashmap index number of the key.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.<% className %>();
    * // add a key/dat pair
    * h.put(<% key1 %>, <% val1 %>);
    * // get key id of <% key1 %>
    * var key = h.keyId(<% key1 %>); // returns 0
    */
    //# exports.<% className %>.prototype.keyId = function(n) { return <% defaultKey %>; }
    JsDeclareFunction(keyId);

	/**
	* Returns n-th dat.
	* @param {number} n - Hashmap dat index number. Should be between 0 and length-1.
	* @returns {<% datType %>} The n-th data value.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.<% className %>();
    * // add a key/dat pair
    * h.put(<% key1 %>, <% val1 %>);
    * // get the first dat
    * var key = h.key(0); // returns <% val1 %>
	*/
    //# exports.<% className %>.prototype.dat = function(n) { return <% defaultVal %>; }
    JsDeclareFunction(dat);

	/**
	* Loads the hashtable from input stream.
	* @param {module:fs.FIn} fin - Input stream.
	* @returns {module:ht.<% className %>} Self.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * var fs = qm.fs;
    * // create a new hashtable
    * var h = new ht.<% className %>();
    * fout = fs.openWrite('map.dat'); // open write stream
    * h.save(fout).close(); // save and close write stream
    * var fin = fs.openRead('map.dat'); // open read stream
    * var h2 = new ht.<% className %>();
    * h2.load(fin); // load
	*/
    //# exports.<% className %>.prototype.load = function(fin) { return this; }
    JsDeclareFunction(load);

	/**
	* Saves the hashtable to output stream.
	* @param {module:fs.FOut} fout - Output stream.
	* @returns {module:fs.FOut} fout.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * var fs = qm.fs;
    * // create a new hashtable
    * var h = new ht.<% className %>();
    * fout = fs.openWrite('map.dat'); // open write stream
    * h.save(fout).close(); // save and close write stream
	*/
    //# exports.<% className %>.prototype.save = function(fout) { return Object.create(require('qminer').fs.FOut.prototype); }
    JsDeclareFunction(save);

	/**
	* Sorts by keys.
	* @param {boolean} [asc=true] - If true, sorts in ascending order.
	* @returns {module:ht.<% className %>} Self.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.<% className %>();
    * h.put(<% key1 %>, <% val1 %>);
    * h.put(<% key2 %>, <% val2 %>);
    * // sort the hashtable by keys
    * h.sortKey();
	*/
	//# exports.<% className %>.prototype.sortKey = function(asc) { return this; }
	JsDeclareFunction(sortKey);

	/**
	* Sorts by dat.
	* @param {boolean} [asc=true] - If true, sorts in ascending order.
	* @returns {module:ht.<% className %>} Self.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var ht = qm.ht;
    * // create a new hashtable
    * var h = new ht.<% className %>();
    * h.put(<% key1 %>, <% val1 %>);
    * h.put(<% key2 %>, <% val2 %>);
    * // sort the hashtable by dat
    * h.sortDat();
	*/
	//# exports.<% className %>.prototype.sortDat = function(asc) { return this; }
	JsDeclareFunction(sortDat);


    static TStr GetClassId() { return TAux::ClassId; }
private:
    static v8::Persistent<v8::Function> constructor;
};

template<class TKey, class TDat, class TAux>
v8::Persistent<v8::Function> TNodeJsHash<TKey, TDat, TAux>::constructor;


typedef TNodeJsHash<TStr, TStr, TAuxStrStrH> TNodeJsStrStrH;
typedef TNodeJsHash<TStr, TInt, TAuxStrIntH> TNodeJsStrIntH;
typedef TNodeJsHash<TStr, TFlt, TAuxStrFltH> TNodeJsStrFltH;
typedef TNodeJsHash<TInt, TStr, TAuxIntStrH> TNodeJsIntStrH;
typedef TNodeJsHash<TInt, TInt, TAuxIntIntH> TNodeJsIntIntH;
typedef TNodeJsHash<TInt, TFlt, TAuxIntFltH> TNodeJsIntFltH;

template<class TKey, class TDat, class TAux>
v8::Local<v8::Object> TNodeJsHash<TKey, TDat, TAux>::WrapInst(v8::Local<v8::Object> Obj) {
	auto Object = new TNodeJsHash<TKey, TDat, TAux>();
	Object->Wrap(Obj);
	return Obj;
}

template<class TKey, class TDat, class TAux>
v8::Local<v8::Object> TNodeJsHash<TKey, TDat, TAux>::WrapInst(v8::Local<v8::Object> Obj, TSIn& SIn) {
	auto Object = new TNodeJsHash<TKey, TDat, TAux>(SIn);
	Object->Wrap(Obj);
	return Obj;
}


template<class TKey, class TDat, class TAux>
void TNodeJsHash<TKey, TDat, TAux>::Init(v8::Local<v8::Object> exports) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> context = Nan::GetCurrentContext();

    TStr Name;
    if (GetClassId() == TNodeJsStrStrH::GetClassId()) { Name = "StrStrMap"; }
    else if (GetClassId() == TNodeJsStrIntH::GetClassId()) { Name = "StrIntMap"; }
    else if (GetClassId() == TNodeJsStrFltH::GetClassId()) { Name = "StrFltMap"; }
    else if (GetClassId() == TNodeJsIntStrH::GetClassId()) { Name = "IntStrMap"; }
    else if (GetClassId() == TNodeJsIntIntH::GetClassId()) { Name = "IntIntMap"; }
    else if (GetClassId() == TNodeJsIntFltH::GetClassId()) { Name = "IntFltMap"; }

    EAssertR(!Name.Empty(), "Could not resolve class ID!");

    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);
    tpl->SetClassName(TNodeJsUtil::ToLocal(Nan::New(Name.CStr())));
    // ObjectWrap uses the first internal field to store the wrapped pointer
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Add all prototype methods, getters and setters here
    NODE_SET_PROTOTYPE_METHOD(tpl, "get", _get);
    NODE_SET_PROTOTYPE_METHOD(tpl, "put", _put);
    NODE_SET_PROTOTYPE_METHOD(tpl, "hasKey", _hasKey);
    NODE_SET_PROTOTYPE_METHOD(tpl, "key", _key);
    NODE_SET_PROTOTYPE_METHOD(tpl, "keyId", _keyId);
    NODE_SET_PROTOTYPE_METHOD(tpl, "dat", _dat);
    NODE_SET_PROTOTYPE_METHOD(tpl, "load", _load);
    NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);
	NODE_SET_PROTOTYPE_METHOD(tpl, "sortKey", _sortKey);
	NODE_SET_PROTOTYPE_METHOD(tpl, "sortDat", _sortDat);

    #if NODE_MODULE_VERSION >= 134 // Node.js >= 24
    tpl->InstanceTemplate()->SetNativeDataProperty(TNodeJsUtil::ToLocal(Nan::New("length")), _length);
    #else
    tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("length")), _length);
    #endif

    // This has to be last, otherwise the properties won't show up on the
    // object in JavaScript
    constructor.Reset(Isolate, TNodeJsUtil::ToLocal(tpl->GetFunction(context)));

    Nan::Set(exports, TNodeJsUtil::ToLocal(Nan::New(Name.CStr())),
        TNodeJsUtil::ToLocal(tpl->GetFunction(context)));
}

template<class TKey, class TDat, class TAux>
void TNodeJsHash<TKey, TDat, TAux>::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.IsConstructCall(), "Trying to initialize HashMap without calling the constructor!");

    if (Args.Length() > 0 && (Args[0]->IsExternal() || Args[0]->IsString())) {
    	PSIn SIn = Args[0]->IsExternal() ?
    			ObjectWrap::Unwrap<TNodeJsFIn>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])))->SIn :
				TFIn::New(TNodeJsUtil::GetArgStr(Args, 0));

    	Args.GetReturnValue().Set(WrapInst(JS_GET_HOLDER(Args), *SIn));
    } else {
    	Args.GetReturnValue().Set(WrapInst(JS_GET_HOLDER(Args)));
    }
}

template<class TKey, class TDat, class TAux>
void TNodeJsHash<TKey, TDat, TAux>::get(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1, "Expected a key as the argument.");
    TNodeJsHash<TKey, TDat>* JsMap = ObjectWrap::Unwrap<TNodeJsHash<TKey, TDat> >(JS_GET_HOLDER(Args));
    TKey Key = TAux::GetArgKey(Args, 0);
    TDat Dat;
    if (JsMap->Map.IsKeyGetDat(Key, Dat)) {
        Args.GetReturnValue().Set(TAux::WrapDat(Dat));
    } else {
        Args.GetReturnValue().Set(Nan::Null());
    }
}

template<class TKey, class TDat, class TAux>
void TNodeJsHash<TKey, TDat, TAux>::put(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 2, "Expected two arguments: a key and a datum.");
    TNodeJsHash<TKey, TDat, TAux>* JsMap = ObjectWrap::Unwrap<TNodeJsHash<TKey, TDat, TAux> >(JS_GET_HOLDER(Args));
    TKey Key = TAux::GetArgKey(Args, 0);
    TDat Dat = TAux::GetArgDat(Args, 1);
    JsMap->Map.AddDat(Key, Dat);
    Args.GetReturnValue().Set(JS_GET_HOLDER(Args));
}

template<class TKey, class TDat, class TAux>
void TNodeJsHash<TKey, TDat, TAux>::hasKey(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1, "Expected a key as the argument.");
    TNodeJsHash<TKey, TDat, TAux>* JsMap = ObjectWrap::Unwrap<TNodeJsHash<TKey, TDat, TAux> >(JS_GET_HOLDER(Args));
    TKey Key = TAux::GetArgKey(Args, 0);
    Args.GetReturnValue().Set(Nan::New(JsMap->Map.IsKey(Key)));
}

template<class TKey, class TDat, class TAux>
void TNodeJsHash<TKey, TDat, TAux>::key(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1, "Expected an index as the argument.");
    TNodeJsHash<TKey, TDat, TAux>* JsMap = ObjectWrap::Unwrap<TNodeJsHash<TKey, TDat, TAux> >(JS_GET_HOLDER(Args));

    const int Idx = TNodeJsUtil::GetArgInt32(Args, 0);
    EAssertR(JsMap->Map.IsKeyId(Idx), TStr::Fmt("%s::key Incorrect KeyId:%d", TAux::ClassId.CStr(), Idx));
    Args.GetReturnValue().Set(TAux::WrapKey(JsMap->Map.GetKey(Idx)));
}

template<class TKey, class TDat, class TAux>
void TNodeJsHash<TKey, TDat, TAux>::keyId(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1, "Expected key as the argument.");
    TNodeJsHash<TKey, TDat, TAux>* JsMap = ObjectWrap::Unwrap<TNodeJsHash<TKey, TDat, TAux> >(JS_GET_HOLDER(Args));

    TKey Key = TAux::GetArgKey(Args, 0);
    int Idx = -1;
    if (JsMap->Map.IsKey(Key)) {
        Idx = JsMap->Map.GetKeyId(Key);
    }
    Args.GetReturnValue().Set(v8::Integer::New(Isolate, Idx));
}

template<class TKey, class TDat, class TAux>
void TNodeJsHash<TKey, TDat, TAux>::dat(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1, "Expected an index as the argument.");
    TNodeJsHash<TKey, TDat, TAux>* JsMap = ObjectWrap::Unwrap<TNodeJsHash<TKey, TDat, TAux> >(JS_GET_HOLDER(Args));

    const int Idx = TNodeJsUtil::GetArgInt32(Args, 0);
    EAssertR(JsMap->Map.IsKeyId(Idx), TStr::Fmt("%s::dat Incorrect KeyId:%d", TAux::ClassId.CStr(), Idx));
    TKey Key;
    TDat Dat;
    JsMap->Map.GetKeyDat(Idx, Key, Dat);
    Args.GetReturnValue().Set(TAux::WrapDat(Dat));
}

template<class TKey, class TDat, class TAux>
void TNodeJsHash<TKey, TDat, TAux>::load(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1 && Args[0]->IsObject(),
        "Expected a TFIn object as the argument.");
    TNodeJsHash<TKey, TDat, TAux>* JsMap = ObjectWrap::Unwrap<TNodeJsHash<TKey, TDat, TAux> >(JS_GET_HOLDER(Args));
    TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])));
    PSIn SIn = JsFIn->SIn;
    JsMap->Map.Load(*SIn);
    Args.GetReturnValue().Set(JS_GET_HOLDER(Args));
}

template<class TKey, class TDat, class TAux>
void TNodeJsHash<TKey, TDat, TAux>::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1 && Args[0]->IsObject(),
        "Expected a TFOut object as the argument.");
    TNodeJsHash<TKey, TDat, TAux>* JsMap = ObjectWrap::Unwrap<TNodeJsHash<TKey, TDat, TAux> >(JS_GET_HOLDER(Args));
    TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])));
    EAssertR(!JsFOut->SOut.Empty(), "Output stream closed!");
    PSOut SOut = JsFOut->SOut;
    JsMap->Map.Save(*SOut);
    Args.GetReturnValue().Set(Args[0]);
}

template<class TKey, class TDat, class TAux>
void TNodeJsHash<TKey, TDat, TAux>::sortKey(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsHash<TKey, TDat, TAux>* JsMap = ObjectWrap::Unwrap<TNodeJsHash<TKey, TDat, TAux> >(JS_GET_HOLDER(Args));
	bool Asc = TNodeJsUtil::GetArgBool(Args, 0, true);
	JsMap->Map.SortByKey(Asc);
	Args.GetReturnValue().Set(JS_GET_HOLDER(Args));
}

template<class TKey, class TDat, class TAux>
void TNodeJsHash<TKey, TDat, TAux>::sortDat(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsHash<TKey, TDat, TAux>* JsMap = ObjectWrap::Unwrap<TNodeJsHash<TKey, TDat, TAux> >(JS_GET_HOLDER(Args));
	bool Asc = TNodeJsUtil::GetArgBool(Args, 0, true);
	JsMap->Map.SortByDat(Asc);
	Args.GetReturnValue().Set(JS_GET_HOLDER(Args));
}

template<class TKey, class TDat, class TAux>
void TNodeJsHash<TKey, TDat, TAux>::length(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    TNodeJsHash<TKey, TDat, TAux>* JsMap = ObjectWrap::Unwrap<TNodeJsHash<TKey, TDat, TAux> >(JS_GET_HOLDER(Info));
    Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsMap->Map.Len()));
}

#endif


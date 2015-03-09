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
    static v8::Handle<v8::Value> WrapKey(const TStr& Val) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope EscapableHandleScope(Isolate);
        return EscapableHandleScope.Escape(v8::String::NewFromUtf8(Isolate, Val.CStr()));
    }
    static TInt GetArgDat(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
        // TJsBase is arbitrary here
        return TNodeJsUtil::GetArgInt32(Args, ArgN);
    }
    static v8::Handle<v8::Value> WrapDat(const int& Val) {
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
    static v8::Handle<v8::Value> WrapKey(const TStr& Val) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope EscapableHandleScope(Isolate);
        return EscapableHandleScope.Escape(v8::String::NewFromUtf8(Isolate, Val.CStr()));
    }
    static TFlt GetArgDat(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
        // TJsBase is arbitrary here
        return TNodeJsUtil::GetArgFlt(Args, ArgN);
    }
    static v8::Handle<v8::Value> WrapDat(const double& Val) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope EscapableHandleScope(Isolate);
        return EscapableHandleScope.Escape(v8::Number::New(Isolate, Val));
    }
};

class TAuxStrStrH {
public:
    static const TStr ClassId; //ClassId is set to "TStrStrH"
    static TStr GetArgKey(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
        // TJsBase is arbitrary here
        return TNodeJsUtil::GetArgStr(Args, ArgN);
    }
    static v8::Handle<v8::Value> WrapKey(const TStr& Val) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope EscapableHandleScope(Isolate);
        return EscapableHandleScope.Escape(v8::String::NewFromUtf8(Isolate, Val.CStr()));
    }
    static TStr GetArgDat(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
        // TJsBase is arbitrary here
        return TNodeJsUtil::GetArgStr(Args, ArgN);
    }
    static v8::Handle<v8::Value> WrapDat(const TStr& Val) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope EscapableHandleScope(Isolate);
        return EscapableHandleScope.Escape(v8::String::NewFromUtf8(Isolate, Val.CStr()));
    }
};

class TAuxIntIntH {
public:
    static const TStr ClassId; //ClassId is set to "TIntIntH"
    static TInt GetArgKey(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
        // TJsBase is arbitrary here
        return TNodeJsUtil::GetArgInt32(Args, ArgN);
    }
    static v8::Handle<v8::Value> WrapKey(const TInt& Val) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope EscapableHandleScope(Isolate);
        return EscapableHandleScope.Escape(v8::Int32::New(Isolate, Val));
    }
    static TInt GetArgDat(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
        // TJsBase is arbitrary here
        return TNodeJsUtil::GetArgInt32(Args, ArgN);
    }
    static v8::Handle<v8::Value> WrapDat(const int& Val) {
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
    static v8::Handle<v8::Value> WrapKey(const TInt& Val) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope EscapableHandleScope(Isolate);
        return EscapableHandleScope.Escape(v8::Int32::New(Isolate, Val));
    }
    static TFlt GetArgDat(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
        // TJsBase is arbitrary here
        return TNodeJsUtil::GetArgFlt(Args, ArgN);
    }
    static v8::Handle<v8::Value> WrapDat(const double& Val) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope EscapableHandleScope(Isolate);
        return EscapableHandleScope.Escape(v8::Number::New(Isolate, Val));
    }
};

class TAuxIntStrH {
public:
    static const TStr ClassId; //ClassId is set to "TIntStrH"
    static TInt GetArgKey(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
        // TJsBase is arbitrary here
        return TNodeJsUtil::GetArgInt32(Args, ArgN);
    }
    static v8::Handle<v8::Value> WrapKey(const TInt& Val) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope EscapableHandleScope(Isolate);
        return EscapableHandleScope.Escape(v8::Int32::New(Isolate, Val));
    }
    static TStr GetArgDat(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
        // TJsBase is arbitrary here
        return TNodeJsUtil::GetArgStr(Args, ArgN);
    }
    static v8::Handle<v8::Value> WrapDat(const TStr& Val) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope EscapableHandleScope(Isolate);
        return EscapableHandleScope.Escape(v8::String::NewFromUtf8(Isolate, Val.CStr()));
    }
};




/**
* Hashtable module.
* @module ht
*/

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

    static void Init(v8::Handle<v8::Object> exports);

    static TKeyDatH& GetMap(const v8::Handle<v8::Object> Obj) {
        TNodeJsHash<TKey, TDat, TAux>* JsMap = ObjectWrap::Unwrap<TNodeJsHash<TKey, TDat, TAux> >(Obj);
        return JsMap->Map;
    }
    static void SetMap(const v8::Handle<v8::Object> Obj, const TKeyDatH& _Map) {
        TNodeJsHash<TKey, TDat, TAux>* JsMap = ObjectWrap::Unwrap<TNodeJsHash<TKey, TDat, TAux> >(Obj);
        JsMap->Map = _Map;
    }

    /**
	* String-int hashmap.
	* @classdesc See {@link module:ht.StrStrMap}. The only difference are the key and data types.
	* @class
	*/
	//# exports.StrIntMap = function() {}
	
    /**
	* String-float hashmap
	* @classdesc See {@link module:ht.StrStrMap}. The only difference are the key and data types.
	* @class
	*/
	//# exports.StrFltMap = function() {}
	
    /**
	* Int-string hashmap
	* @classdesc See {@link module:ht.StrStrMap}. The only difference are the key and data types.
	* @class
	*/
	//# exports.IntStrMap = function() {}
	
    /**
	* Int-int hashmap
	* @classdesc See {@link module:ht.StrStrMap}. The only difference are the key and data types.
	* @class
	*/
	//# exports.IntIntMap = function() {}
	
    /**
	* Int-float hashmap
	* @classdesc See {@link module:ht.StrStrMap}. The only difference are the key and data types.
	* @class
	*/
	//# exports.IntFltMap = function() {}
    
	/**
	* String-string hashmap	
	* @class
	*/
	//# exports.StrStrMap = function() {}
	JsDeclareFunction(New);	
	
	/**
	* Returns dat given key
	* @param {string} key - Hashmap key.
	* @returns {string} data - Hashmap data.
	*/
    //# exports.StrStrMap.prototype.get = function(key) {}
    JsDeclareFunction(get);
	
	/**
	* add/update key-value pair
	* @param {string} key - Hashmap key.
	* @param {string} data - Hashmap data.
	* @returns {module:ht.StrStrMap} this - Returns self.
	*/
    //# exports.StrStrMap.prototype.put = function(key, data) {}
    JsDeclareFunction(put);    
	
	/**
	* returns true if the map has a given key 
	* @param {string} key - Hashmap key.	
	* @returns {boolean} haskey - Returns true if the map contains key.
	*/
    //# exports.StrStrMap.prototype.hasKey = function(key) {}
    JsDeclareFunction(hasKey);
	
	/**
    * @property {number} length Number of key/dat pairs
	*/
	//# exports.StrStrMap.prototype.length = undefined;
	JsDeclareProperty(length);
    
	/**
	* returns n-th key
	* @param {number} n - Hashmap key number.	
	* @returns {string} key - Returns n-th key.
	*/
    //# exports.StrStrMap.prototype.key = function(n) {}	
    JsDeclareFunction(key);

	/**
	* returns n-th dat
	* @param {number} n - Hashmap dat number.	
	* @returns {string} dat - Returns n-th data value.
	*/
    //# exports.StrStrMap.prototype.dat = function(n) {}    
    JsDeclareFunction(dat);
    
	/**
	* loads the hashtable from input stream
	* @param {module:fs.FIn} fin - Input stream.	
	* @returns {module:ht.StrStrMap} map - Returns map.
	*/
    //# exports.StrStrMap.prototype.load = function(fin) {}  	
    JsDeclareFunction(load);

	/**
	* saves the hashtable to output stream
	* @param {module:fs.FOut} fout - Output stream.	
	* @returns {module:fs.FOut} fout - Returns the input parameter (so close can be called if needed).
	*/
    //# exports.StrStrMap.prototype.save = function(fout) {}  	    
    JsDeclareFunction(save);

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
void TNodeJsHash<TKey, TDat, TAux>::Init(v8::Handle<v8::Object> exports) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();

    TStr Name;
    if (GetClassId() == TNodeJsStrStrH::GetClassId()) { Name = "StrStrMap"; }
    else if (GetClassId() == TNodeJsStrIntH::GetClassId()) { Name = "StrIntMap"; }
    else if (GetClassId() == TNodeJsStrFltH::GetClassId()) { Name = "StrFltMap"; }
    else if (GetClassId() == TNodeJsIntStrH::GetClassId()) { Name = "IntStrMap"; }
    else if (GetClassId() == TNodeJsIntIntH::GetClassId()) { Name = "IntIntMap"; }
    else if (GetClassId() == TNodeJsIntFltH::GetClassId()) { Name = "IntFltMap"; }

    EAssertR(!Name.Empty(), "Could not resolve class ID!");

    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);
    tpl->SetClassName(v8::String::NewFromUtf8(Isolate, Name.CStr()));
    // ObjectWrap uses the first internal field to store the wrapped pointer
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Add all prototype methods, getters and setters here
    NODE_SET_PROTOTYPE_METHOD(tpl, "get", _get);
    NODE_SET_PROTOTYPE_METHOD(tpl, "put", _put);
    NODE_SET_PROTOTYPE_METHOD(tpl, "hasKey", _hasKey);
    NODE_SET_PROTOTYPE_METHOD(tpl, "key", _key);
    NODE_SET_PROTOTYPE_METHOD(tpl, "dat", _dat);
    NODE_SET_PROTOTYPE_METHOD(tpl, "load", _load);
    NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "length"), _length);

    // This has to be last, otherwise the properties won't show up on the
    // object in JavaScript
    constructor.Reset(Isolate, tpl->GetFunction());
    exports->Set(v8::String::NewFromUtf8(Isolate, Name.CStr()),
        tpl->GetFunction());
}

template<class TKey, class TDat, class TAux>
void TNodeJsHash<TKey, TDat, TAux>::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.IsConstructCall(), "Trying to initialize HashMap without calling the constructor!");

    if (Args.Length() > 0 && (Args[0]->IsExternal() || Args[0]->IsString())) {
    	PSIn SIn = Args[0]->IsExternal() ?
    			ObjectWrap::Unwrap<TNodeJsFIn>(Args[0]->ToObject())->SIn :
				TFIn::New(TNodeJsUtil::GetArgStr(Args, 0));

    	Args.GetReturnValue().Set(WrapInst(Args.Holder(), *SIn));
    } else {
    	Args.GetReturnValue().Set(WrapInst(Args.Holder()));
    }
}

template<class TKey, class TDat, class TAux>
void TNodeJsHash<TKey, TDat, TAux>::get(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1, "Expected a key as the argument.");
    TNodeJsHash<TKey, TDat>* JsMap = ObjectWrap::Unwrap<TNodeJsHash<TKey, TDat> >(Args.Holder());
    TKey Key = TAux::GetArgKey(Args, 0);
    TDat Dat;
    if (JsMap->Map.IsKeyGetDat(Key, Dat)) {
        Args.GetReturnValue().Set(TAux::WrapDat(Dat));
    } else {
        Args.GetReturnValue().Set(v8::Null(Isolate));
    }
}

template<class TKey, class TDat, class TAux>
void TNodeJsHash<TKey, TDat, TAux>::put(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 2, "Expected two arguments: a key and a datum.");
    TNodeJsHash<TKey, TDat, TAux>* JsMap = ObjectWrap::Unwrap<TNodeJsHash<TKey, TDat, TAux> >(Args.Holder());
    TKey Key = TAux::GetArgKey(Args, 0);
    TDat Dat = TAux::GetArgDat(Args, 1);
    JsMap->Map.AddDat(Key, Dat);
    Args.GetReturnValue().Set(Args.Holder());
}

template<class TKey, class TDat, class TAux>
void TNodeJsHash<TKey, TDat, TAux>::hasKey(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1, "Expected a key as the argument.");
    TNodeJsHash<TKey, TDat, TAux>* JsMap = ObjectWrap::Unwrap<TNodeJsHash<TKey, TDat, TAux> >(Args.Holder());
    TKey Key = TAux::GetArgKey(Args, 0);
    Args.GetReturnValue().Set(v8::Boolean::New(Isolate, JsMap->Map.IsKey(Key)));
}

template<class TKey, class TDat, class TAux>
void TNodeJsHash<TKey, TDat, TAux>::key(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1, "Expected an index as the argument.");
    TNodeJsHash<TKey, TDat, TAux>* JsMap = ObjectWrap::Unwrap<TNodeJsHash<TKey, TDat, TAux> >(Args.Holder());

    const int Idx = TNodeJsUtil::GetArgInt32(Args, 0);
    EAssertR(JsMap->Map.IsKeyId(Idx), TStr::Fmt("%s::key Incorrect KeyId:%d", TAux::ClassId.CStr(), Idx));
    Args.GetReturnValue().Set(TAux::WrapKey(JsMap->Map.GetKey(Idx)));
}

template<class TKey, class TDat, class TAux>
void TNodeJsHash<TKey, TDat, TAux>::dat(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1, "Expected an index as the argument.");
    TNodeJsHash<TKey, TDat, TAux>* JsMap = ObjectWrap::Unwrap<TNodeJsHash<TKey, TDat, TAux> >(Args.Holder());

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
    TNodeJsHash<TKey, TDat, TAux>* JsMap = ObjectWrap::Unwrap<TNodeJsHash<TKey, TDat, TAux> >(Args.Holder());
    TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Args[0]->ToObject());
    PSIn SIn = JsFIn->SIn;
    JsMap->Map.Load(*SIn);
    Args.GetReturnValue().Set(Args.Holder());
}

template<class TKey, class TDat, class TAux>
void TNodeJsHash<TKey, TDat, TAux>::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1 && Args[0]->IsObject(),
        "Expected a TFOut object as the argument.");
    TNodeJsHash<TKey, TDat, TAux>* JsMap = ObjectWrap::Unwrap<TNodeJsHash<TKey, TDat, TAux> >(Args.Holder());
    TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args[0]->ToObject());
    PSOut SOut = JsFOut->SOut;
    JsMap->Map.Save(*SOut);
    Args.GetReturnValue().Set(Args[0]);
}

template<class TKey, class TDat, class TAux>
void TNodeJsHash<TKey, TDat, TAux>::length(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    TNodeJsHash<TKey, TDat, TAux>* JsMap = ObjectWrap::Unwrap<TNodeJsHash<TKey, TDat, TAux> >(Info.Holder());
    Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsMap->Map.Len()));
}

#endif


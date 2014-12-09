#ifndef QMINER_SNAP_NODEJS
#define QMINER_SNAP_NODEJS

#define BUILDING_NODE_EXTENSION

#include <node.h>
#include <node_object_wrap.h>
//#include "base.h"
#include "Snap.h"

// NOTE: This is *not* the same as in QMiner JS. 
#define JsDeclareProperty(Function) \
	static void Function(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info); \
	static void _ ## Function(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) { \
	   v8::Isolate* Isolate = v8::Isolate::GetCurrent(); \
	   v8::HandleScope HandleScope(Isolate); \
	   try { \
	      Function(Name, Info); \
	   } catch (const PExcept& Except) { \
	      /* if(typeid(Except) == typeid(TQmExcept::New(""))) { */ \
            Isolate->ThrowException(v8::Exception::TypeError( \
               v8::String::NewFromUtf8(Isolate, "[la addon] Exception"))); \
         /* } else { \
            throw Except; \
         } */ \
	   } \
	}

#define JsDeclareFunction(Function) \
   static void Function(const v8::FunctionCallbackInfo<v8::Value>& Args); \
   static void _ ## Function(const v8::FunctionCallbackInfo<v8::Value>& Args) { \
      v8::Isolate* Isolate = v8::Isolate::GetCurrent(); \
      v8::HandleScope HandleScope(Isolate); \
      try { \
         Function(Args); \
      } catch (const PExcept& Except) { \
         /* if(typeid(Except) == typeid(TQmExcept::New(""))) { */ \
            Isolate->ThrowException(v8::Exception::TypeError( \
               v8::String::NewFromUtf8(Isolate, "[la addon] Exception"))); \
         /* } else { \
            throw Except; \
         } */ \
      } \
   }

// XXX: The macro expects that variables Args and Isolate exist. 
#define QmAssert(Cond) \
   if (!(Cond)) { \
      Args.GetReturnValue().Set(Isolate->ThrowException(v8::Exception::TypeError( \
         v8::String::NewFromUtf8(Isolate, "[la addon] Exception")))); return; }

// XXX: The macro expects that variable Args and Isolate exist. 
#define QmAssertR(Cond, MsgStr) \
  if (!(Cond)) { \
   Args.GetReturnValue().Set(Isolate->ThrowException(v8::Exception::TypeError( \
         v8::String::NewFromUtf8(Isolate, MsgStr)))); return; }

// XXX: The macro expects that variables Args and Isolate exist. 
#define QmFailR(MsgStr) \
   Args.GetReturnValue().Set(Isolate->ThrowException(v8::Exception::TypeError( \
         v8::String::NewFromUtf8(Isolate, MsgStr)))); return;

class TNodeJsUtil {
public:
   /// Extract argument ArgN property as bool
	static bool GetArgBool(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& Property, const bool& DefVal) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);
		if (Args.Length() > ArgN) {
			if (Args[ArgN]->IsObject() && Args[ArgN]->ToObject()->Has(v8::String::NewFromUtf8(Isolate, Property.CStr()))) {
				v8::Handle<v8::Value> Val = Args[ArgN]->ToObject()->Get(v8::String::NewFromUtf8(Isolate, Property.CStr()));
				 EAssertR(Val->IsBoolean(),
				   TStr::Fmt("Argument %d, property %s expected to be boolean", ArgN, Property.CStr()).CStr());
				 return static_cast<bool>(Val->BooleanValue());
			}
		}
		return DefVal;
	}
   static int GetArgInt32(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& Property, const int& DefVal) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);
		if (Args.Length() > ArgN) {			
			if (Args[ArgN]->IsObject() && Args[ArgN]->ToObject()->Has(v8::String::NewFromUtf8(Isolate, Property.CStr()))) {
				v8::Handle<v8::Value> Val = Args[ArgN]->ToObject()->Get(v8::String::NewFromUtf8(Isolate, Property.CStr()));
				 EAssertR(Val->IsInt32(),
				   TStr::Fmt("Argument %d, property %s expected to be int32", ArgN, Property.CStr()).CStr());
				 return Val->ToNumber()->Int32Value();
			}
		}
		return DefVal;
	}
	// gets the class name of the underlying glib object. the name is stored in an hidden variable "class"
	static TStr GetClass(const v8::Handle<v8::Object> Obj) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);
		v8::Local<v8::Value> ClassNm = Obj->GetHiddenValue(v8::String::NewFromUtf8(Isolate, "class"));
		const bool EmptyP = ClassNm.IsEmpty();
		if (EmptyP) { return ""; }
		v8::String::Utf8Value Utf8(ClassNm);
		return TStr(*Utf8);		
	}

	// checks if the class name of the underlying glib object matches the given string. the name is stored in an hidden variable "class"
	static bool IsClass(const v8::Handle<v8::Object> Obj, const TStr& ClassNm) {
		TStr ObjClassStr = GetClass(Obj);		
		return ObjClassStr == ClassNm;
	}
	/// Check if argument ArgN belongs to a given class
	static bool IsArgClass(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& ClassNm) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);
		EAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d of class %s", ArgN, ClassNm.CStr()).CStr());
      EAssertR(Args[ArgN]->IsObject(), TStr("Argument expected to be '" + ClassNm + "' but is not even an object!").CStr());
		v8::Handle<v8::Value> Val = Args[ArgN];
	 	v8::Handle<v8::Object> Data = v8::Handle<v8::Object>::Cast(Val);
		TStr ClassStr = GetClass(Data);
		return ClassStr.EqI(ClassNm);
	}
};

///////////////////////////////
// NodeJs-Qminer-Snap
// XXX: Current implementation doesn't support all documented properties! 

class TNodeJsSnap : public node::ObjectWrap {
public:
	//# 
	//# **Functions and properties:**
	//# 
	//#- `graph = snap.newUGraph()` -- generate an empty undirected graph
private:
	//static v8::Persistent<v8::Function> constructor;
};

///////////////////////////////
// NodeJs-Qminer-Graph

template <class T>
class TNodeJsGraph : public node::ObjectWrap {
public:
	TPt<T> Graph;
	static void Init(v8::Handle<v8::Object> exports);
	static v8::Local<v8::Object> New();
public:
	TNodeJsGraph() { Graph = T::New(); }
public:
	//# 
	//# **Functions and properties:**
	JsDeclareFunction(New);
private:
	JsDeclareFunction(addNode);
	JsDeclareFunction(addEdge);
	JsDeclareFunction(delNode);
	JsDeclareFunction(delEdge);
	JsDeclareFunction(isNode);
	JsDeclareFunction(isEdge);
	JsDeclareProperty(nodes);
	JsDeclareProperty(edges);
	JsDeclareFunction(node);
	JsDeclareFunction(firstNode);
	JsDeclareFunction(lastNode);
private:
	static v8::Persistent<v8::Function> constructor;
};

///////////////////////////////
// NodeJs-Qminer-Node

template <class T>
class TNodeJsNode : public node::ObjectWrap {
public:
	typename T::TNodeI Node;

	static void Init(v8::Handle<v8::Object> exports);
	static v8::Local<v8::Object> New(const typename T::TNodeI Node);
	
public:
	TNodeJsNode() {}
	TNodeJsNode(typename T::TNodeI node) { Node = node; }
public:
	//# 
	//# **Functions and properties:**
	JsDeclareFunction(New);
	JsDeclareProperty(id);
	JsDeclareProperty(deg);
	JsDeclareProperty(inDeg);
	JsDeclareProperty(outDeg);
	JsDeclareFunction(nbrId);
	JsDeclareFunction(prev);
	JsDeclareFunction(next);
private:

private:
	static v8::Persistent<v8::Function> constructor;
};

#endif
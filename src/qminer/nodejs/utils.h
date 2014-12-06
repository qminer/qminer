#ifndef QMINER_NODEJS_UTILS
#define QMINER_NODEJS_UTILS

#include <node.h>
#include "base.h"

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
               v8::String::NewFromUtf8(Isolate, "[addon] Exception"))); \
         /* } else { \
            throw Except; \
         } */ \
	   } \
	}

#define JsDeclareSetProperty(GetFunction, SetFunction) \
	static void GetFunction(const v8::FunctionCallbackInfo<v8::Value>& Args); \
	static void _ ## GetFunction(const v8::FunctionCallbackInfo<v8::Value>& Args) { \
		v8::Isolate* Isolate = v8::Isolate::GetCurrent(); \
		v8::HandleScope HandleScope(Isolate); \
		try { \
			GetFunction(Args); \
		} catch (const PExcept& Except) { \
            Isolate->ThrowException(v8::Exception::TypeError( \
               v8::String::NewFromUtf8(Isolate, "[addon] Exception"))); \
      } \
	} \
	static void SetFunction(const v8::FunctionCallbackInfo<v8::Value>& Args); \
	static void _ ## SetFunction(const v8::FunctionCallbackInfo<v8::Value>& Args) { \
		v8::Isolate* Isolate = v8::Isolate::GetCurrent(); \
		v8::HandleScope HandleScope(Isolate); \
		try { \
			SetFunction(Args); \
		} catch (const PExcept& Except) { \
            Isolate->ThrowException(v8::Exception::TypeError( \
               v8::String::NewFromUtf8(Isolate, "[addon] Exception"))); \
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
	     Isolate->ThrowException(v8::Exception::TypeError(\
		 v8::String::NewFromUtf8(Isolate, TStr("[addon] Exception: " + Except->GetMsgStr()).CStr()))); \
	     /* } else { \
	     throw Except; \
	     } */ \
	  } \
   }




//////////////////////////////////////////////////////
// Node - Utilities
class TNodeJsUtil {
public:

	/// Extract argument ArgN property as bool
	static bool GetArgBool(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);

		EAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		EAssertR(Val->IsBoolean(), TStr::Fmt("Argument %d expected to be bool", ArgN));
		return static_cast<bool>(Val->BooleanValue());		
	}
	/// Extract argument ArgN property as bool, and use DefVal in case when not present
	static bool GetArgBool(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const bool& DefVal) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);
		
		EAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		if (Val->IsBoolean()) {
			return static_cast<bool>(Val->BooleanValue());
		}
		else {
			return DefVal;
		}
	}
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

	/// Extract argument ArgN as int
	static bool GetArgInt32(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);

		EAssertR(Args.Length() > ArgN, TStr::Fmt("TNodeJsUtil::GetArgInt32: Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		EAssertR(Val->IsInt32(), TStr::Fmt("Argument %d expected to be int", ArgN));
		return static_cast<bool>(Val->Int32Value());		
	}
	/// Extract argument ArgN as int, and use DefVal in case when not present
	static int GetArgInt32(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const int& DefVal) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);

		EAssertR(Args.Length() > ArgN, TStr::Fmt("TNodeJsUtil::GetArgInt32: Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		if (Val->IsInt32()) {
			return static_cast<bool>(Val->Int32Value());
		}
		else {
			return DefVal;
		}
	}
	/// Extract argument ArgN property as int
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

   /// Extract argument ArgN as TStr
   static TStr GetArgStr(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
	   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	   v8::HandleScope HandleScope(Isolate);
	   EAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
	   v8::Handle<v8::Value> Val = Args[ArgN];
	   EAssertR(Val->IsString(), TStr::Fmt("Argument %d expected to be string", ArgN));
	   v8::String::Utf8Value Utf8(Val);
	   return TStr(*Utf8);
   }
   /// Extract argument ArgN as TStr, and use DefVal in case when not present
   static TStr GetArgStr(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& DefVal) {
	   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);
		if (Args.Length() > ArgN) {
			v8::Handle<v8::Value> Val = Args[ArgN];
			EAssertR(Val->IsString(), TStr::Fmt("Argument %d expected to be string", ArgN));
			v8::String::Utf8Value Utf8(Val);
			return TStr(*Utf8);
		}
		return DefVal;
	}
	/// Extract argument ArgN property as string
	static TStr GetArgStr(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& Property, const TStr& DefVal) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);
		if (Args.Length() > ArgN) {
			if (Args[ArgN]->IsObject() && Args[ArgN]->ToObject()->Has(v8::String::NewFromUtf8(Isolate, Property.CStr()))) {
				v8::Handle<v8::Value> Val = Args[ArgN]->ToObject()->Get(v8::String::NewFromUtf8(Isolate, Property.CStr()));
				EAssertR(Val->IsString(), TStr::Fmt("Argument %d, property %s expected to be string", ArgN, Property.CStr()));
				v8::String::Utf8Value Utf8(Val);
				return TStr(*Utf8);
			}
		}
		return DefVal;
	}

	static PJsonVal GetArgJson(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);
	static v8::Local<v8::Value> ParseJson(v8::Isolate* Isolate, const PJsonVal& JsonVal);

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
		EAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d of class %s", ArgN, ClassNm.CStr()));		
		EAssertR(Args[ArgN]->IsObject(), TStr("Argument expected to be '" + ClassNm + "' but is not even an object!"));		
		v8::Handle<v8::Value> Val = Args[ArgN];
	 	v8::Handle<v8::Object> Data = v8::Handle<v8::Object>::Cast(Val);
		TStr ClassStr = GetClass(Data);
		return ClassStr.EqI(ClassNm);
	}

	static bool IsArgFun(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);

		EAssertR(Args.Length() > ArgN, TStr::Fmt("Missing function argument %d", ArgN).CStr());

		v8::Handle<v8::Value> Val = Args[ArgN];
		return Val->IsFunction();
	}

private:
	static PJsonVal GetObjJson(const v8::Local<v8::Object>& Obj);
};

#endif

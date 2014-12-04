#include "qm_nodejs.h"

using namespace TQm;

///////////////////////////////
// NodeJs-Qminer-Base
TWPt<TQm::TBase> TNodeJsBase::Base = nullptr;

void TNodeJsBase::Init(v8::Handle<v8::Object> exports) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   // Add all methods, getters and setters here.
   NODE_SET_METHOD(exports, "open", open);
   NODE_SET_METHOD(exports, "close", close);
   NODE_SET_METHOD(exports, "store", store);
   NODE_SET_METHOD(exports, "getStoreList", getStoreList);
   NODE_SET_METHOD(exports, "createStore", createStore);
   NODE_SET_METHOD(exports, "search", search);
   NODE_SET_METHOD(exports, "gc", gc);
   NODE_SET_METHOD(exports, "newStreamAggr", newStreamAggr);
   NODE_SET_METHOD(exports, "getStreamAggr", getStreamAggr);
   NODE_SET_METHOD(exports, "getStreamAggrNames", getStreamAggrNames);

}

void TNodeJsBase::open(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	//Args.GetReturnValue().Set(v8::Number::New(Isolate, Sum));
}

void TNodeJsBase::close(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	//Args.GetReturnValue().Set(v8::Number::New(Isolate, Sum));
}

void TNodeJsBase::store(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   //Args.GetReturnValue().Set(v8::Number::New(Isolate, Sum));
}

void TNodeJsBase::getStoreList(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	//Args.GetReturnValue().Set(v8::Number::New(Isolate, Sum));
}

void TNodeJsBase::createStore(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   //Args.GetReturnValue().Set(v8::Number::New(Isolate, Sum));
}

void TNodeJsBase::search(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   //Args.GetReturnValue().Set(v8::Number::New(Isolate, Sum));
}

void TNodeJsBase::gc(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   Base->GarbageCollect();   
   Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsBase::newStreamAggr(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
      
   //Args.GetReturnValue().Set(v8::Number::New(Isolate, Sum));
}

void TNodeJsBase::getStreamAggr(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   //Args.GetReturnValue().Set(v8::Number::New(Isolate, Sum));
}

void TNodeJsBase::getStreamAggrNames(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   //Args.GetReturnValue().Set(v8::Number::New(Isolate, Sum));
}



///////////////////////////////
// NodeJs-Qminer-Store
v8::Persistent<v8::Function> TNodeJsStore::constructor;

void TNodeJsStore::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "store"));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all prototype methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "rec", rec);
	NODE_SET_PROTOTYPE_METHOD(tpl, "each", each);
	NODE_SET_PROTOTYPE_METHOD(tpl, "map", map);
	NODE_SET_PROTOTYPE_METHOD(tpl, "add", add);
	NODE_SET_PROTOTYPE_METHOD(tpl, "newRec", newRec);
	NODE_SET_PROTOTYPE_METHOD(tpl, "newRecSet", newRecSet);
	NODE_SET_PROTOTYPE_METHOD(tpl, "sample", sample);
	NODE_SET_PROTOTYPE_METHOD(tpl, "field", field);
	NODE_SET_PROTOTYPE_METHOD(tpl, "key", key);
	NODE_SET_PROTOTYPE_METHOD(tpl, "addTrigger", addTrigger);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getStreamAggr", getStreamAggr);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getStreamAggrNames", getStreamAggrNames);
	NODE_SET_PROTOTYPE_METHOD(tpl, "toJSON", toJSON);
	NODE_SET_PROTOTYPE_METHOD(tpl, "clear", clear);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getVec", getVec);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getMat", getMat);
	NODE_SET_PROTOTYPE_METHOD(tpl, "cell", cell);

	// Properties 
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "name"), name);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "empty"), empty);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "length"), length);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "recs"), recs);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "fields"), fields);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "joins"), joins);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "keys"), keys);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "first"), first);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "last"), last);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "forwardIter"), forwardIter);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "backwardIter"), backwardIter);


	// This has to be last, otherwise the properties won't show up on the
	// object in JavaScript.
	constructor.Reset(Isolate, tpl->GetFunction());
	exports->Set(v8::String::NewFromUtf8(Isolate, "store"),
		tpl->GetFunction());
}

v8::Local<v8::Object> TNodeJsStore::New(TWPt<TQm::TStore> _Store) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);

	v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
	v8::Local<v8::Object> Instance = cons->NewInstance();	

	TNodeJsStore* JsStore = new TNodeJsStore(_Store);
	JsStore->Wrap(Instance);
	return HandleScope.Escape(Instance);
}

void TNodeJsStore::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	if (Args.Length() > 0) {
		TStr StoreNm = TNodeJsUtil::GetArgStr(Args, 0);
		if (TNodeJsBase::Base->IsStoreNm(StoreNm)) {
			TWPt<TQm::TStore> Store = TNodeJsBase::Base->GetStoreByStoreNm(StoreNm);
			Args.GetReturnValue().Set(TNodeJsStore::New(Store));
			return;
		}
	}
	else {
		 Args.GetReturnValue().Set(v8::Undefined(Isolate));
	}	
}

void TNodeJsStore::rec(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	
	try {
		TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Args.Holder());

		const TStr RecNm = TNodeJsUtil::GetArgStr(Args, 0);
		if (JsStore->Store->IsRecNm(RecNm)) {
			Args.GetReturnValue().Set(TNodeJsRec::New(JsStore->Store->GetRec(RecNm)));
		} else {
			Args.GetReturnValue().Set(v8::Undefined(Isolate));
		}
	} catch (const PExcept& Except) {
		printf("Failed to fetch record: %s\n", Except->GetMsgStr().CStr());
		Args.GetReturnValue().Set(v8::Undefined(Isolate));
	}
}

void TNodeJsStore::each(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(TNodeJsUtil::IsArgFun(Args, 0), "each: Argument 0 should be a function!");
	Args.GetReturnValue().Set(Args.Holder());

	v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);

	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Args.Holder());
	const TWPt<TQm::TStore> Store = JsStore->Store;

	if (!Store->Empty()) {
		TQm::PStoreIter Iter = Store->ForwardIter();

		const unsigned Argc = 1;

		TJsRec Rec

		uint64 Count = 0;
		while (Iter->Next()) {
			const uint64 RecId = Iter->GetRecId();

			Local<Value> ArgV[Argc] = { Local<Number>::New(v8::Integer::New(Count++)) };
			Callback->Call(Context::GetCurrent()->Global(), Argc, ArgV);
		}
	}
}

void TNodeJsStore::map(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	//Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
}

void TNodeJsStore::add(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	//Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
}

void TNodeJsStore::newRec(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	//Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
}

void TNodeJsStore::newRecSet(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	//Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
}

void TNodeJsStore::sample(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	//Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
}

void TNodeJsStore::field(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	//Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
}

void TNodeJsStore::key(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	//Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
}

void TNodeJsStore::addTrigger(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	//Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
}

void TNodeJsStore::getStreamAggr(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	//Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
}

void TNodeJsStore::getStreamAggrNames(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	//Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
}

void TNodeJsStore::toJSON(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	//Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
}

void TNodeJsStore::clear(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	//Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
}

void TNodeJsStore::getVec(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	//Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
}

void TNodeJsStore::getMat(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	//Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
}

void TNodeJsStore::cell(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	//Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
}

void TNodeJsStore::name(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	//Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsMat->Mat.GetCols()));
}

void TNodeJsStore::empty(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	//Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsMat->Mat.GetCols()));
}

void TNodeJsStore::length(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	//Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsMat->Mat.GetCols()));
}

void TNodeJsStore::recs(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	//Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsMat->Mat.GetCols()));
}

void TNodeJsStore::fields(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	//Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsMat->Mat.GetCols()));
}

void TNodeJsStore::joins(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	//Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsMat->Mat.GetCols()));
}

void TNodeJsStore::keys(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	//Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsMat->Mat.GetCols()));
}

void TNodeJsStore::first(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	//Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsMat->Mat.GetCols()));
}

void TNodeJsStore::last(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	//Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsMat->Mat.GetCols()));
}

void TNodeJsStore::forwardIter(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	//Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsMat->Mat.GetCols()));
}

void TNodeJsStore::backwardIter(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	//Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsMat->Mat.GetCols()));
}

///////////////////////////////
// Register functions, etc.  
void init(v8::Handle<v8::Object> exports) {
   TNodeJsBase::Init(exports);   
   TNodeJsStore::Init(exports);
}

NODE_MODULE(qm, init)


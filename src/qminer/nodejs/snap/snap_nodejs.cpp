#include "snap_nodejs.h"

///////////////////////////////
// NodeJs-Qminer-Snap 
//
/*
v8::Persistent<v8::Function> TNodeJsSnap::constructor;

void TNodeJsSnap::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

   // Add all prototype methods, getters and setters here.
   //NODE_SET_METHOD(exports, "hello", hello);
}
*/
 ///////////////////////////////
 // NodeJs-Qminer-Graph 
 //

template <class T>
v8::Persistent<v8::Function> TNodeJsGraph<T>::constructor;

template <>
void TNodeJsGraph<TUNGraph>::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);

	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "ugraph"));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all prototype methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "addNode", _addNode);
	NODE_SET_PROTOTYPE_METHOD(tpl, "addEdge", _addEdge);
	NODE_SET_PROTOTYPE_METHOD(tpl, "delNode", _delNode);
	NODE_SET_PROTOTYPE_METHOD(tpl, "delEdge", _delEdge);
	NODE_SET_PROTOTYPE_METHOD(tpl, "isNode", _isNode);
	NODE_SET_PROTOTYPE_METHOD(tpl, "isEdge", _isEdge);
	NODE_SET_PROTOTYPE_METHOD(tpl, "node", _node);
	NODE_SET_PROTOTYPE_METHOD(tpl, "firstNode", _firstNode);
	NODE_SET_PROTOTYPE_METHOD(tpl, "lastNode", _lastNode);

	// Properties
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "nodes"), _nodes);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "edges"), _edges);

	// This has to be last, otherwise the properties won't show up on the
	// object in JavaScript.
	constructor.Reset(Isolate, tpl->GetFunction());
	exports->Set(v8::String::NewFromUtf8(Isolate, "ugraph"), tpl->GetFunction());
}

template <>
void TNodeJsGraph<TNGraph>::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);

	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "dgraph"));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all prototype methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "addNode", _addNode);
	NODE_SET_PROTOTYPE_METHOD(tpl, "addEdge", _addEdge);
	NODE_SET_PROTOTYPE_METHOD(tpl, "delNode", _delNode);
	NODE_SET_PROTOTYPE_METHOD(tpl, "delEdge", _delEdge);
	NODE_SET_PROTOTYPE_METHOD(tpl, "isNode", _isNode);
	NODE_SET_PROTOTYPE_METHOD(tpl, "isEdge", _isEdge);
	NODE_SET_PROTOTYPE_METHOD(tpl, "node", _node);
	NODE_SET_PROTOTYPE_METHOD(tpl, "firstNode", _firstNode);
	NODE_SET_PROTOTYPE_METHOD(tpl, "firstNode", _lastNode);

	// Properties
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "nodes"), _nodes);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "edges"), _edges);

	// This has to be last, otherwise the properties won't show up on the
	// object in JavaScript.
	constructor.Reset(Isolate, tpl->GetFunction());
	exports->Set(v8::String::NewFromUtf8(Isolate, "dgraph"), tpl->GetFunction());
}

template <>
void TNodeJsGraph<TNEGraph>::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);

	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "dmgraph"));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all prototype methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "addNode", _addNode);
	NODE_SET_PROTOTYPE_METHOD(tpl, "addEdge", _addEdge);
	NODE_SET_PROTOTYPE_METHOD(tpl, "delNode", _delNode);
	NODE_SET_PROTOTYPE_METHOD(tpl, "delEdge", _delEdge);
	NODE_SET_PROTOTYPE_METHOD(tpl, "isNode", _isNode);
	NODE_SET_PROTOTYPE_METHOD(tpl, "isEdge", _isEdge);
	NODE_SET_PROTOTYPE_METHOD(tpl, "node", _node);
	NODE_SET_PROTOTYPE_METHOD(tpl, "firstNode", _firstNode);
	NODE_SET_PROTOTYPE_METHOD(tpl, "firstNode", _lastNode);

	// Properties
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "nodes"), _nodes);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "edges"), _edges);

	// This has to be last, otherwise the properties won't show up on the
	// object in JavaScript.
	constructor.Reset(Isolate, tpl->GetFunction());
	exports->Set(v8::String::NewFromUtf8(Isolate, "dmgraph"), tpl->GetFunction());
}

template <class T>
v8::Local<v8::Object> TNodeJsGraph<T>::New(){
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);

	v8::Local<v8::Function> Cons = v8::Local<v8::Function>::New(Isolate, constructor);
	v8::Local<v8::Object> Instance = Cons->NewInstance();

	TNodeJsGraph<T>* NodeGraph = new TNodeJsGraph<T>();
	NodeGraph->Wrap(Instance);
	return HandleScope.Escape(Instance);
};

template <class T>
void TNodeJsGraph<T>::New(const v8::FunctionCallbackInfo<v8::Value>& Args){
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);
	if (Args.IsConstructCall()) {
		TNodeJsGraph<T>* NodeGraph = new TNodeJsGraph<T>();
		v8::Local<v8::Object> Instance = Args.This();
		NodeGraph->Wrap(Instance);
		Args.GetReturnValue().Set(Instance);
	}
	else {
		const int Argc = 1;
		v8::Local<v8::Value> Argv[Argc] = { Args[0] };
		v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
		cons->NewInstance(Argc, Argv);
		v8::Local<v8::Object> Instance = cons->NewInstance(Argc, Argv);
		Args.GetReturnValue().Set(Instance);
	}
};

template <class T>
void TNodeJsGraph<T>::addNode(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	
	v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
	v8::Local<v8::Object> Instance = cons->NewInstance();

	v8::Local<v8::Object> Self = Args.Holder();
	TNodeJsGraph* NodeJsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(Self);

	if (Args.Length() != 1) {
		Isolate->ThrowException(v8::Exception::TypeError(
			v8::String::NewFromUtf8(Isolate, "Expected 1 argument.")));
	}
	else if (!Args[0]->IsNumber()) {
		Isolate->ThrowException(v8::Exception::TypeError(
			v8::String::NewFromUtf8(Isolate, "Expected number")));
	}
	else {
		int id = Args[0]->ToNumber()->Value();
		if (!NodeJsGraph->Graph->IsNode(id))
			NodeJsGraph->Graph->AddNode(id);
	}	
}

template <class T>
void TNodeJsGraph<T>::addEdge(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Args.Holder();
	TNodeJsGraph* NodeJsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(Self);

	if (Args.Length() != 2) {
		Isolate->ThrowException(v8::Exception::TypeError(
			v8::String::NewFromUtf8(Isolate, "Expected 2 arguments.")));
	}
	else {
		int SrcId = Args[0]->ToNumber()->Value();
		int DstId = Args[1]->ToNumber()->Value();
		if (NodeJsGraph->Graph->IsNode(SrcId) && NodeJsGraph->Graph->IsNode(DstId))
			NodeJsGraph->Graph->AddEdge(SrcId, DstId);
	}
}

template <class T>
void TNodeJsGraph<T>::delNode(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
	v8::Local<v8::Object> Instance = cons->NewInstance();

	v8::Local<v8::Object> Self = Args.Holder();
	TNodeJsGraph* NodeJsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(Self);

	if (Args.Length() != 1) {
		Isolate->ThrowException(v8::Exception::TypeError(
			v8::String::NewFromUtf8(Isolate, "Expected 1 argument.")));
	}
	else if (!Args[0]->IsNumber()) {
		Isolate->ThrowException(v8::Exception::TypeError(
			v8::String::NewFromUtf8(Isolate, "Expected number")));
	}
	else {
		int id = Args[0]->ToNumber()->Value();
		if (NodeJsGraph->Graph->IsNode(id))
			NodeJsGraph->Graph->DelNode(id);
	}
}

template <class T>
void TNodeJsGraph<T>::delEdge(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Args.Holder();
	TNodeJsGraph* NodeJsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(Self);

	if (Args.Length() != 2) {
		Isolate->ThrowException(v8::Exception::TypeError(
			v8::String::NewFromUtf8(Isolate, "Expected 2 arguments.")));
	}
	else {
		int SrcId = Args[0]->ToNumber()->Value();
		int DstId = Args[1]->ToNumber()->Value();
		if (NodeJsGraph->Graph->IsEdge(SrcId, DstId))
			NodeJsGraph->Graph->DelEdge(SrcId, DstId);
	}
}

template <class T>
void TNodeJsGraph<T>::isNode(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	
	v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
	v8::Local<v8::Object> Instance = cons->NewInstance();

	v8::Local<v8::Object> Self = Args.Holder();
	TNodeJsGraph* NodeJsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(Self);

	bool node = false;

	if (Args.Length() != 1) {
		Isolate->ThrowException(v8::Exception::TypeError(
			v8::String::NewFromUtf8(Isolate, "Expected 1 argument.")));
		Args.GetReturnValue().Set(v8::Boolean::New(Isolate, node));
	}
	else if (!Args[0]->IsNumber()) {
		Isolate->ThrowException(v8::Exception::TypeError(
			v8::String::NewFromUtf8(Isolate, "Expected number")));
		Args.GetReturnValue().Set(v8::Boolean::New(Isolate, node));
	}
	else {
		int id = Args[0]->ToNumber()->Value();
		node = NodeJsGraph->Graph->IsNode(id);
		Args.GetReturnValue().Set(v8::Boolean::New(Isolate, node));
	}
}

template <class T>
void TNodeJsGraph<T>::isEdge(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Args.Holder();
	TNodeJsGraph* NodeJsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(Self);

	bool edge = false;

	if (Args.Length() != 2) {
		Isolate->ThrowException(v8::Exception::TypeError(
			v8::String::NewFromUtf8(Isolate, "Expected 2 arguments.")));
		Args.GetReturnValue().Set(v8::Boolean::New(Isolate, edge));
	}
	else {
		int SrcId = Args[0]->ToNumber()->Value();
		int DstId = Args[1]->ToNumber()->Value();
		edge = NodeJsGraph->Graph->IsEdge(SrcId, DstId);
		Args.GetReturnValue().Set(v8::Boolean::New(Isolate, edge));
	}
}

template <class T>
void TNodeJsGraph<T>::firstNode(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Args.Holder();
	TNodeJsGraph* NodeJsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(Self);
	T::TNodeI ReturnNode = NodeJsGraph->Graph->BegNI();
	Args.GetReturnValue().Set(TNodeJsNode<T>::New(ReturnNode));
}

template <class T>
void TNodeJsGraph<T>::lastNode(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Args.Holder();
	TNodeJsGraph* NodeJsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(Self);
	T::TNodeI ReturnNode = NodeJsGraph->Graph->EndNI();
	Args.GetReturnValue().Set(TNodeJsNode<T>::New(ReturnNode));
}

template <class T>
void TNodeJsGraph<T>::node(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	int id = Args[0]->ToNumber()->Value();

	v8::Local<v8::Object> Self = Args.Holder();
	TNodeJsGraph* NodeJsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(Self);
	T::TNodeI ReturnNode = NodeJsGraph->Graph->GetNI(id);
	Args.GetReturnValue().Set(TNodeJsNode<T>::New(ReturnNode));
}

template <class T>
void TNodeJsGraph<T>::nodes(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsGraph* JsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(Info.Holder());
	int nodes = JsGraph->Graph->GetNodes();
	Info.GetReturnValue().Set(v8::Number::New(Isolate, nodes));
}

template <class T>
void TNodeJsGraph<T>::edges(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsGraph* JsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(Info.Holder());
	int edges = JsGraph->Graph->GetEdges();
	Info.GetReturnValue().Set(v8::Number::New(Isolate, edges));
}

template <class T>
v8::Persistent<v8::Function> TNodeJsNode<T>::constructor;

template <class T>
void TNodeJsNode<T>::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "node"));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
	// Add all prototype methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "nbrId", _nbrId);
	NODE_SET_PROTOTYPE_METHOD(tpl, "next", _next);
	NODE_SET_PROTOTYPE_METHOD(tpl, "prev", _prev);

	// Properties 
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "id"), _id);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "deg"), _deg);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "inDeg"), _inDeg);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "outDeg"), _outDeg);

	constructor.Reset(Isolate, tpl->GetFunction());
	exports->Set(v8::String::NewFromUtf8(Isolate, "node"),
		tpl->GetFunction());
}


template <class T>
v8::Local<v8::Object> TNodeJsNode<T>::New(const typename T::TNodeI node) {

	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);

	v8::Local<v8::Function> Cons = v8::Local<v8::Function>::New(Isolate, constructor);
	v8::Local<v8::Object> Instance = Cons->NewInstance();

	TNodeJsNode<T>* JsNode = new TNodeJsNode<T>(node); // calls: TNodeJsNode(typename T::TNodeI node, int a) { Node = node; }
	JsNode->Wrap(Instance);

	return HandleScope.Escape(Instance);
};

template <class T>
void TNodeJsNode<T>::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);
	v8::Local<v8::Object> Instance = Args.This();
	Args.GetReturnValue().Set(Instance);
};

template <class T>
void TNodeJsNode<T>::id(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsNode* JsNode = ObjectWrap::Unwrap<TNodeJsNode>(Info.Holder());
	int id = JsNode->Node.GetId();
	Info.GetReturnValue().Set(v8::Number::New(Isolate, id));
}

template <class T>
void TNodeJsNode<T>::deg(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsNode* JsNode = ObjectWrap::Unwrap<TNodeJsNode>(Info.Holder());
	int degree = JsNode->Node.GetDeg();
	Info.GetReturnValue().Set(v8::Number::New(Isolate, degree));
}

template <class T>
void TNodeJsNode<T>::inDeg(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsNode* JsNode = ObjectWrap::Unwrap<TNodeJsNode>(Info.Holder());
	int degree = JsNode->Node.GetInDeg();
	Info.GetReturnValue().Set(v8::Number::New(Isolate, degree));
}

template <class T>
void TNodeJsNode<T>::outDeg(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsNode* JsNode = ObjectWrap::Unwrap<TNodeJsNode>(Info.Holder());
	int degree = JsNode->Node.GetOutDeg();
	Info.GetReturnValue().Set(v8::Number::New(Isolate, degree));
}

template <class T>
void TNodeJsNode<T>::nbrId(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	int N = Args[0]->ToNumber()->Value();
	TNodeJsNode* JsNode = ObjectWrap::Unwrap<TNodeJsNode>(Args.Holder());
	int nbrid = JsNode->Node.GetNbrNId(N);
	Args.GetReturnValue().Set(v8::Number::New(Isolate, nbrid));
}

template <class T>
void TNodeJsNode<T>::next(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	int N = Args[0]->ToNumber()->Value();
	TNodeJsNode* JsNode = ObjectWrap::Unwrap<TNodeJsNode>(Args.Holder());
	JsNode->Node++;
	return Args.GetReturnValue().Set(JsNode);
}

template <class T>
void TNodeJsNode<T>::prev(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	int N = Args[0]->ToNumber()->Value();
	TNodeJsNode* JsNode = ObjectWrap::Unwrap<TNodeJsNode>(Args.Holder());
	JsNode->Node--;
	return Args.GetReturnValue().Set(JsNode);
}

///////////////////////////////
// Register functions, etc.
void init(v8::Handle<v8::Object> exports) {
	//TNodeJsSnap::Init(exports);
	TNodeJsGraph<TUNGraph>::Init(exports);
	TNodeJsGraph<TNGraph>::Init(exports);
	TNodeJsGraph<TNEGraph>::Init(exports);
	TNodeJsNode<TUNGraph>::Init(exports);
	TNodeJsNode<TNGraph>::Init(exports);
	TNodeJsNode<TNEGraph>::Init(exports);
}
NODE_MODULE(snap, init)
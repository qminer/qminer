#include "snap_nodejs.h"
#include "../la/la_nodejs.h"

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
	NODE_SET_PROTOTYPE_METHOD(tpl, "eachNode", _eachNode);
	NODE_SET_PROTOTYPE_METHOD(tpl, "eachEdge", _eachEdge);
	NODE_SET_PROTOTYPE_METHOD(tpl, "adjMat", _adjMat);
	NODE_SET_PROTOTYPE_METHOD(tpl, "dump", _dump);
	NODE_SET_PROTOTYPE_METHOD(tpl, "components", _components);

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
	NODE_SET_PROTOTYPE_METHOD(tpl, "lastNode", _lastNode);
	NODE_SET_PROTOTYPE_METHOD(tpl, "eachNode", _eachNode);
	NODE_SET_PROTOTYPE_METHOD(tpl, "eachEdge", _eachEdge);
	NODE_SET_PROTOTYPE_METHOD(tpl, "adjMat", _adjMat);
	NODE_SET_PROTOTYPE_METHOD(tpl, "dump", _dump);
	NODE_SET_PROTOTYPE_METHOD(tpl, "components", _components);

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
	NODE_SET_PROTOTYPE_METHOD(tpl, "lastNode", _lastNode);
	NODE_SET_PROTOTYPE_METHOD(tpl, "eachNode", _eachNode);
	NODE_SET_PROTOTYPE_METHOD(tpl, "eachEdge", _eachEdge);
	NODE_SET_PROTOTYPE_METHOD(tpl, "adjMat", _adjMat);
	NODE_SET_PROTOTYPE_METHOD(tpl, "dump", _dump);
	NODE_SET_PROTOTYPE_METHOD(tpl, "components", _components);

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
	if (NodeJsGraph->Graph->IsNode(id)) {
		T::TNodeI ReturnNode = NodeJsGraph->Graph->GetNI(id);
		Args.GetReturnValue().Set(TNodeJsNode<T>::New(ReturnNode));
	}
	else {
		return Args.GetReturnValue().Set(NULL);
	}

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
void TNodeJsGraph<T>::eachNode(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsGraph* JsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(Args.Holder());

	v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);
	const unsigned Argc = 2;
	int Count = 0;
	for (typename T::TNodeI NI = JsGraph->Graph->BegNI(); NI < JsGraph->Graph->EndNI(); NI++)
	{
		v8::Local<v8::Object> NodeObj = TNodeJsNode<T>::New(NI);
		v8::Local<v8::Value> ArgV[Argc] = {
			NodeObj, v8::Local<v8::Number>::New(Isolate, v8::Integer::NewFromUnsigned(Isolate, Count++))
		};
		Callback->Call(Isolate->GetCurrentContext()->Global(), Argc, ArgV);
		Count++;
	}

	Args.GetReturnValue().Set(JsGraph);
}

template <class T>
void TNodeJsGraph<T>::eachEdge(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsGraph* JsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(Args.Holder());

	v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);
	const unsigned Argc = 2;
	int Count = 0;
	for (typename T::TEdgeI EI = JsGraph->Graph->BegEI(); EI < JsGraph->Graph->EndEI(); EI++)
	{
		v8::Local<v8::Object> EdgeObj = TNodeJsEdge<T>::New(EI);
		v8::Local<v8::Value> ArgV[Argc] = {
			EdgeObj, v8::Local<v8::Number>::New(Isolate, v8::Integer::NewFromUnsigned(Isolate, Count++))
		};
		Callback->Call(Isolate->GetCurrentContext()->Global(), Argc, ArgV);
		Count++;
	}

	Args.GetReturnValue().Set(JsGraph);
}

template <class T>
void TNodeJsGraph<T>::adjMat(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsGraph* JsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(Args.Holder());

	int Nodes = JsGraph->Graph->GetNodes();
	TVec<TIntFltKdV> Mat(Nodes);

	THash<TInt, THash<TInt, TInt> > MultiGraph;

	TIntSet NIdSet(JsGraph->Graph->GetNodes()); // remapping
	// build the remapping of all keys
	for (typename T::TNodeI NI = JsGraph->Graph->BegNI(); NI < JsGraph->Graph->EndNI(); NI++) {
		int NId = NI.GetId();
		NIdSet.AddKey(NId);
	}
	// count outgoing edges, remap ids and build the sparse ajdacency matrix
	for (typename T::TNodeI NI = JsGraph->Graph->BegNI(); NI < JsGraph->Graph->EndNI(); NI++) {
		int NId = NI.GetId();
		int RemappedNId = NIdSet.GetKeyId(NId);
		int OutDeg = NI.GetOutDeg();
		TIntIntH Neigh(OutDeg);
		MultiGraph.AddDat(NId, Neigh);
		// take all outgoing edges and increment or add
		for (int k = 0; k < OutDeg; k++) {
			int OutNId = NI.GetOutNId(k);
			if (MultiGraph.GetDat(NId).IsKey(OutNId)) {
				MultiGraph.GetDat(NId).GetDat(OutNId)++;
			}
			else {
				MultiGraph.GetDat(NId).AddDat(OutNId, 1);
			}
		}

		int Len = MultiGraph.GetDat(NId).Len();
		Mat[RemappedNId].Gen(Len);
		for (int k = 0; k < Len; k++) {
			int Key = MultiGraph.GetDat(NId).GetKey(k);
			Mat[RemappedNId][k].Key = NIdSet.GetKeyId(Key);
			Mat[RemappedNId][k].Dat = MultiGraph.GetDat(NId).GetDat(Key);
		}
		Mat[RemappedNId].Sort();
	}

	Args.GetReturnValue().Set(TNodeJsSpMat::New(Mat));
}

template <class T>
void TNodeJsGraph<T>::dump(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsGraph* JsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(Args.Holder());

	v8::String::Utf8Value str(Args[0]->ToString());
	TStr FNm = *str;

	if (FNm != "") {
		FILE* pFile = fopen(FNm.CStr(), "w");
		JsGraph->Graph->Dump(pFile);
		fclose(pFile);
	}
	else {
		JsGraph->Graph->Dump(); // how to dump this out?
	}
	
	Args.GetReturnValue().Set(Args.Holder());
}

template <class T>
void TNodeJsGraph<T>::components(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsGraph* JsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(Args.Holder());
	bool IsWeak = Args[0]->BooleanValue();

	TCnComV CnComV;
	if (IsWeak) {
		TSnap::GetWccs(JsGraph->Graph, CnComV);
	}
	else {
		TSnap::GetSccs(JsGraph->Graph, CnComV);
	}

	TVec<TIntFltKdV> Mat(CnComV.Len());
	for (int i = 0; i < CnComV.Len(); i++) {
		Mat[i].Gen(CnComV[i].Len());
		for (int j = 0; j < CnComV[i].Len(); j++) {
			int id = CnComV[i][j];
			Mat[i][j].Key = id;
			Mat[i][j].Dat = 1;
		}
	}

	Args.GetReturnValue().Set(TNodeJsSpMat::New(Mat));
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
	NODE_SET_PROTOTYPE_METHOD(tpl, "eachNbr", _eachNbr);
	NODE_SET_PROTOTYPE_METHOD(tpl, "eachInNbr", _eachInNbr);
	NODE_SET_PROTOTYPE_METHOD(tpl, "eachOutNbr", _eachOutNbr);
	NODE_SET_PROTOTYPE_METHOD(tpl, "eachEdge", _eachEdge);
	NODE_SET_PROTOTYPE_METHOD(tpl, "eachInEdge", _eachInEdge);
	NODE_SET_PROTOTYPE_METHOD(tpl, "eachOutEdge", _eachOutEdge);

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

template <class T>
void TNodeJsNode<T>::eachNbr(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsNode* JsNode = ObjectWrap::Unwrap<TNodeJsNode>(Args.Holder());
	
	v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);
	const unsigned Argc = 2;
	int Count = 0;
	v8::Local<v8::Value> ArgV;
	for (int i = 0; i < JsNode->Node.GetDeg(); i++) {
		int NbrId = JsNode->Node.GetNbrNId(i);
		v8::Local<v8::Value> ArgV[Argc] = {
			v8::Integer::New(Isolate, NbrId), v8::Local<v8::Number>::New(Isolate, v8::Integer::NewFromUnsigned(Isolate, Count++))
		};
		Callback->Call(Isolate->GetCurrentContext()->Global(), Argc, ArgV);
		Count++;
	}

	Args.GetReturnValue().Set(JsNode);
}

template <class T>
void TNodeJsNode<T>::eachInNbr(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsNode* JsNode = ObjectWrap::Unwrap<TNodeJsNode>(Args.Holder());

	v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);
	const unsigned Argc = 2;
	int Count = 0;
	v8::Local<v8::Value> ArgV;
	for (int i = 0; i < JsNode->Node.GetInDeg(); i++) {
		int NbrId = JsNode->Node.GetInNId(i);
		v8::Local<v8::Value> ArgV[Argc] = {
			v8::Integer::New(Isolate, NbrId), v8::Local<v8::Number>::New(Isolate, v8::Integer::NewFromUnsigned(Isolate, Count++))
		};
		Callback->Call(Isolate->GetCurrentContext()->Global(), Argc, ArgV);
		Count++;
	}

	Args.GetReturnValue().Set(JsNode);
}

template <class T>
void TNodeJsNode<T>::eachOutNbr(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsNode* JsNode = ObjectWrap::Unwrap<TNodeJsNode>(Args.Holder());

	v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);
	const unsigned Argc = 2;
	int Count = 0;
	v8::Local<v8::Value> ArgV;
	for (int i = 0; i < JsNode->Node.GetOutDeg(); i++) {
		int NbrId = JsNode->Node.GetOutNId(i);
		v8::Local<v8::Value> ArgV[Argc] = {
			v8::Integer::New(Isolate, NbrId), v8::Local<v8::Number>::New(Isolate, v8::Integer::NewFromUnsigned(Isolate, Count++))
		};
		Callback->Call(Isolate->GetCurrentContext()->Global(), Argc, ArgV);
		Count++;
	}

	Args.GetReturnValue().Set(JsNode);
}

template <class T>
void TNodeJsNode<T>::eachEdge(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	Args.GetReturnValue().Set(Args.Holder());
}

template <>
void TNodeJsNode<TNEGraph>::eachEdge(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsNode* JsNode = ObjectWrap::Unwrap<TNodeJsNode>(Args.Holder());

	v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);
	const unsigned Argc = 2;
	int Count = 0;

	int Len = JsNode->Node.GetDeg();
	for (int EdgeN = 0; EdgeN < Len; EdgeN++) {
		int NbrEId = JsNode->Node.GetNbrEId(EdgeN);
		v8::Local<v8::Value> ArgV[Argc] = {
			v8::Integer::New(Isolate, NbrEId), v8::Local<v8::Number>::New(Isolate, v8::Integer::NewFromUnsigned(Isolate, Count++))
		};
		Callback->Call(Isolate->GetCurrentContext()->Global(), Argc, ArgV);
		Count++;
	}

	Args.GetReturnValue().Set(JsNode);
}

template <class T>
void TNodeJsNode<T>::eachInEdge(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	Args.GetReturnValue().Set(Args.Holder());
}

template <>
void TNodeJsNode<TNEGraph>::eachInEdge(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsNode* JsNode = ObjectWrap::Unwrap<TNodeJsNode>(Args.Holder());

	v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);
	const unsigned Argc = 2;
	int Count = 0;

	int Len = JsNode->Node.GetInDeg();
	for (int EdgeN = 0; EdgeN < Len; EdgeN++) {
		int NbrEId = JsNode->Node.GetInEId(EdgeN);
		v8::Local<v8::Value> ArgV[Argc] = {
			v8::Integer::New(Isolate, NbrEId), v8::Local<v8::Number>::New(Isolate, v8::Integer::NewFromUnsigned(Isolate, Count++))
		};
		Callback->Call(Isolate->GetCurrentContext()->Global(), Argc, ArgV);
		Count++;
	}

	Args.GetReturnValue().Set(JsNode);
}

template <class T>
void TNodeJsNode<T>::eachOutEdge(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	Args.GetReturnValue().Set(Args.Holder());
}

template <>
void TNodeJsNode<TNEGraph>::eachOutEdge(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsNode* JsNode = ObjectWrap::Unwrap<TNodeJsNode>(Args.Holder());

	v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);
	const unsigned Argc = 2;
	int Count = 0;

	int Len = JsNode->Node.GetOutDeg();
	for (int EdgeN = 0; EdgeN < Len; EdgeN++) {
		int NbrEId = JsNode->Node.GetOutEId(EdgeN);
		v8::Local<v8::Value> ArgV[Argc] = {
			v8::Integer::New(Isolate, NbrEId), v8::Local<v8::Number>::New(Isolate, v8::Integer::NewFromUnsigned(Isolate, Count++))
		};
		Callback->Call(Isolate->GetCurrentContext()->Global(), Argc, ArgV);
		Count++;
	}

	Args.GetReturnValue().Set(JsNode);
}

template <class T>
v8::Persistent<v8::Function> TNodeJsEdge<T>::constructor;

template <class T>
void TNodeJsEdge<T>::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "edge"));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "srcId"), _srcId);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "dstId"), _dstId);

	constructor.Reset(Isolate, tpl->GetFunction());
	exports->Set(v8::String::NewFromUtf8(Isolate, "edge"),
		tpl->GetFunction());
}

template <class T>
v8::Local<v8::Object> TNodeJsEdge<T>::New(const typename T::TEdgeI edge) {

	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);

	v8::Local<v8::Function> Cons = v8::Local<v8::Function>::New(Isolate, constructor);
	v8::Local<v8::Object> Instance = Cons->NewInstance();

	TNodeJsEdge<T>* JsEdge = new TNodeJsEdge<T>(edge); // calls: TNodeJsEdge(typename T::TEdgeI node, int a) { Edge = edge; }
	JsEdge->Wrap(Instance);

	return HandleScope.Escape(Instance);
};

template <class T>
void TNodeJsEdge<T>::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);
	v8::Local<v8::Object> Instance = Args.This();
	Args.GetReturnValue().Set(Instance);
};

template <class T>
void TNodeJsEdge<T>::srcId(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsEdge* JsEdge = ObjectWrap::Unwrap<TNodeJsEdge>(Info.Holder());
	int id = JsEdge->Edge.GetSrcNId();
	Info.GetReturnValue().Set(v8::Number::New(Isolate, id));
}

template <class T>
void TNodeJsEdge<T>::dstId(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsEdge* JsEdge = ObjectWrap::Unwrap<TNodeJsEdge>(Info.Holder());
	int id = JsEdge->Edge.GetDstNId();
	Info.GetReturnValue().Set(v8::Number::New(Isolate, id));
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
	TNodeJsEdge<TUNGraph>::Init(exports);
	TNodeJsEdge<TNGraph>::Init(exports);
	TNodeJsEdge<TNEGraph>::Init(exports);
	// LA
	TNodeJsVec<TFlt, TAuxFltV>::Init(exports);
	TNodeJsVec<TInt, TAuxIntV>::Init(exports);
	TNodeJsFltVV::Init(exports);
	TNodeJsSpVec::Init(exports);
	TNodeJsSpMat::Init(exports);
}
NODE_MODULE(snap, init)
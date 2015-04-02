#include "snap_nodejs.h"


///////////////////////////////
// NodeJs-Qminer-Snap 
//

void TNodeJsSnap::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// Add all methods, getters and setters here.
	NODE_SET_METHOD(exports, "cmtyEvolution", _cmtyEvolution);

}

void TNodeJsSnap::cmtyEvolution(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	printf("cmty evolution alg");
	/*
	int ArgsLen = Args.Length();
	if (ArgsLen == 9 || ArgsLen == 10){

		v8::Handle<v8::Array> Array = v8::Handle<v8::Array>::Cast(Args[0]);
		TVec<PUNGraph, TSize> gs;

		if (ArgsLen == 10) {
			v8::String::Utf8Value str(Args[9]->ToString());
			TStr path = *str;
			TSnap::LoadGraphArray(path, gs);
			for (int i = 0; i < gs.Len(); i++) {
				Array->Set(i, TNodeJsGraph<TUNGraph>::New(gs[i]));
			}
		}
		else {
			for (int i = 0; i<Array->Length(); i++) {
				v8::Local<v8::Object> g = Array->Get(i)->ToObject();
				v8::Local<v8::External> WrappedObject = v8::Local<v8::External>::Cast(g->GetInternalField(0));
				TNodeJsGraph<TUNGraph>* JsGraph = static_cast<TNodeJsGraph<TUNGraph>*>(WrappedObject->Value());
				gs.Add(JsGraph->Graph());
			}
		}

		double alpha = Args[1]->ToNumber()->Value();
		double beta = Args[2]->ToNumber()->Value();

		v8::Local<v8::Object> GraphObj = Args[3]->ToObject();
		TNodeJsGraph<TNGraph>* JsOutGraph = ObjectWrap::Unwrap<TNodeJsGraph<TNGraph>>(GraphObj);
		PNGraph outGraph = JsOutGraph->Graph();
		
		NEED TNodeJsQminer Hash to finish this!!!

		
		TJsHash<TInt, TInt, TAuxIntIntH>* timeHash = TJsObjUtil<TJsHash<TInt, TInt, TAuxIntIntH>>::GetArgObj(Args, 4);
		TIntH& t = timeHash->Map;

		TJsHash<TInt, TInt, TAuxIntIntH>* commHash = TJsObjUtil<TJsHash<TInt, TInt, TAuxIntIntH>>::GetArgObj(Args, 5);
		TIntH& c = commHash->Map;

		TJsHash<TInt, TInt, TAuxIntIntH>* sizeHash = TJsObjUtil<TJsHash<TInt, TInt, TAuxIntIntH>>::GetArgObj(Args, 6);
		TIntH& s = sizeHash->Map;

		TJsIntV* edgeSize = TJsObjUtil<TQm::TJsIntV>::GetArgObj(Args, 7);
		TIntV& e = edgeSize->Vec;

		TJsSpMat* membersMat = TJsObjUtil<TJsSpMat>::GetArgObj(Args, 8);
		TIntIntVH m;

		TSnap::CmtyEvolutionBatchGraph(gs, outGraph, t, c, s, e, m, alpha, beta, 2);

		TVec<TIntFltKdV> Mat(m.Len());

		for (int i = 0; i < m.Len(); i++) {
			Mat[i].Gen(m[i].Len());
			for (int j = 0; j < m[i].Len(); j++) {
				int id = m[i][j];
				Mat[i][j].Key = id;
				Mat[i][j].Dat = 1;
			}
		}

		TVec<TIntFltKdV>& M = membersMat->Mat;
		M = Mat;

		

		Args.GetReturnValue().Set(Args.Holder);
	}*/
	
}

 ///////////////////////////////
 // NodeJs-Qminer-Graph 
 //


template <>
void TNodeJsGraph<TUNGraph>::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);

	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "UndirectedGraph"));
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
	NODE_SET_PROTOTYPE_METHOD(tpl, "renumber", _renumber);
	NODE_SET_PROTOTYPE_METHOD(tpl, "degreeCentrality", _degreeCentrality);
	NODE_SET_PROTOTYPE_METHOD(tpl, "load", _load);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

	// Properties
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "nodes"), _nodes);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "edges"), _edges);

	// This has to be last, otherwise the properties won't show up on the
	// object in JavaScript.
	constructor.Reset(Isolate, tpl->GetFunction());
	exports->Set(v8::String::NewFromUtf8(Isolate, "UndirectedGraph"), tpl->GetFunction());
}

template <>
void TNodeJsGraph<TNGraph>::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);

	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "DirectedGraph"));
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
	NODE_SET_PROTOTYPE_METHOD(tpl, "renumber", _renumber);
	NODE_SET_PROTOTYPE_METHOD(tpl, "degreeCentrality", _degreeCentrality);
	NODE_SET_PROTOTYPE_METHOD(tpl, "load", _load);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

	// Properties
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "nodes"), _nodes);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "edges"), _edges);

	// This has to be last, otherwise the properties won't show up on the
	// object in JavaScript.
	constructor.Reset(Isolate, tpl->GetFunction());
	exports->Set(v8::String::NewFromUtf8(Isolate, "DirectedGraph"), tpl->GetFunction());
}

template <>
void TNodeJsGraph<TNEGraph>::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);

	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "DirectedMultigraph"));
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
	NODE_SET_PROTOTYPE_METHOD(tpl, "renumber", _renumber);
	NODE_SET_PROTOTYPE_METHOD(tpl, "degreeCentrality", _degreeCentrality);
	NODE_SET_PROTOTYPE_METHOD(tpl, "load", _load);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

	// Properties
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "nodes"), _nodes);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "edges"), _edges);

	// This has to be last, otherwise the properties won't show up on the
	// object in JavaScript.
	constructor.Reset(Isolate, tpl->GetFunction());
	exports->Set(v8::String::NewFromUtf8(Isolate, "DirectedMultigraph"), tpl->GetFunction());
}


template <>
inline void TNodeJsNode<TNEGraph>::eachEdge(const v8::FunctionCallbackInfo<v8::Value>& Args) {
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

	Args.GetReturnValue().Set(Args.Holder());
}

template <>
inline void TNodeJsNode<TNEGraph>::eachInEdge(const v8::FunctionCallbackInfo<v8::Value>& Args) {
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

	Args.GetReturnValue().Set(Args.Holder());
}

template <>
inline void TNodeJsNode<TNEGraph>::eachOutEdge(const v8::FunctionCallbackInfo<v8::Value>& Args) {
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

	Args.GetReturnValue().Set(Args.Holder());
}

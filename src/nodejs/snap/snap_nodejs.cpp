/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include "snap_nodejs.h"

///////////////////////////////
// NodeJs-Qminer-Snap 
//

void TNodeJsSnap::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// Add all methods, getters and setters here.
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
        NODE_SET_PROTOTYPE_METHOD(tpl, "clusteringCoefficient", _clusteringCoefficient);
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
        NODE_SET_PROTOTYPE_METHOD(tpl, "clusteringCoefficient", _clusteringCoefficient);
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
    NODE_SET_PROTOTYPE_METHOD(tpl, "clusteringCoefficient", _clusteringCoefficient);
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
	v8::TryCatch TryCatch;
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
		TNodeJsUtil::CheckJSExcept(TryCatch);
		Count++;
	}

	Args.GetReturnValue().Set(Args.Holder());
}

template <>
inline void TNodeJsNode<TNEGraph>::eachInEdge(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	v8::TryCatch TryCatch;
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
		TNodeJsUtil::CheckJSExcept(TryCatch);
		Count++;
	}

	Args.GetReturnValue().Set(Args.Holder());
}

template <>
inline void TNodeJsNode<TNEGraph>::eachOutEdge(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	v8::TryCatch TryCatch;
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
		TNodeJsUtil::CheckJSExcept(TryCatch);
		Count++;
	}

	Args.GetReturnValue().Set(Args.Holder());
}

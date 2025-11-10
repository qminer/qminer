/**g1.eachEdge(function (E) { console.log("("+E.srcId+","+E.dstId+")"});
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef QMINER_SNAP_NODEJS
#define QMINER_SNAP_NODEJS

#include <node.h>
#include <node_object_wrap.h>
#include "base.h"
#include "../nodeutil.h"
#include "../fs/fs_nodejs.h"
#include "../la/la_nodejs.h"
#include "Snap.h"

//#ifndef BUILDING_NODE_EXTENSION
//    #define BUILDING_NODE_EXTENSION
//#endif


/**
* Snap module.
* @module snap
* @example
* // TODO
*/
class TNodeJsSnap : public node::ObjectWrap {
public:
    static void Init(v8::Local<v8::Object> exports);
        // **Functions and properties:**
        //
        // - `ugraph = new snap.UndirectedGraph` - returns undirected graph.
        // - `dgraph = new snap.DirectedGraph` - returns directed graph.
        // - `dmgraph = new snap.DirectedMultigraph` - returns directed multigraph.

private:
};

///////////////////////////////
// NodeJs-Qminer-Graph

template <class T>
class TNodeJsGraph : public node::ObjectWrap {
public:
    TPt<T> Graph;
    static void Init(v8::Local<v8::Object> exports);
    static v8::Local<v8::Object> New();
    static v8::Local<v8::Object> New(TStr path);
    static v8::Local<v8::Object> New(TPt<T> _graph);

public:
    TNodeJsGraph() { Graph = T::New(); };
    TNodeJsGraph(TStr path) { Graph = TSnap::LoadEdgeList<TPt<T>>(path); };
    TNodeJsGraph(TPt<T> _graph) { Graph = _graph; };
public:
    //#
    //# **Functions and properties:**
    JsDeclareFunction(New);
private:
        /**
        * Adds a node to the graph.
        * @param {int} X - id of new edge
        * @example
        * // import the analytics and la modules
        * var snap = require('qminer').snap;
        * // create a new UndirectedGraph object
        * var graph = new snap.UnidrectedGraph;
        * // add a node to the graph
        * graph.addNode(1);
        */
    JsDeclareFunction(addNode);
        /**
        * Adds an edge to the graph.
        * @param {int} X - id of the first node
        * @param {int} Y - id of the second node
        * @example
        * // import the analytics and la modules
        * var snap = require('qminer').snap;
        * // create a new UndirectedGraph object
        * var graph = new snap.UnidrectedGraph;
        * // add the first node to the graph
        * graph.addNode(1);
        * // add the second node to the graph
        * graph.addNode(2);
        * // add an edge
        * graph.addEdge(1,2);
        */
    JsDeclareFunction(addEdge);
        /**
        * Deletes a node of the graph.
        * @param {int} X - id of the node
        * @example
        * // delete an edge
        * graph.delNode(1,2);
        */
    JsDeclareFunction(delNode);
        /**
        * Deletes an edge of the graph.
        * @param {int} X - id of the first node
        * @param {int} Y - id of the second node
        * @example
        * // delete an edge
        * graph.delEdge(1,2);
        */
    JsDeclareFunction(delEdge);
        /**
        * Checks if node exists
        * @param {int} X - id of the node
        * @returns {boolean} true|false.
        * @example
        * // check if node with id 1 exists in the graph
        * graph.isNode(1);
        */
    JsDeclareFunction(isNode);
        /**
        * Checks if an edge exists
        * @param {int} X - id of the first node
        * @param {int} Y - id of the second node
        * @returns {boolean} true|false.
        * @example
        * // check if edge (1,2) exists in the graph
        * graph.isEdge(1,2);
        */
    JsDeclareFunction(isEdge);
        /**
        * Returns number of nodes in the graph
        * @returns {int} N.
        * @example
        * // returns number of nodes in a graph
        * N = graph.nodes
        */
    JsDeclareProperty(nodes);
        /**
        * Returns number of edges in the graph
        * @returns {int} E.
        * @example
        * // returns number of edges in a graph
        * E = graph.edges
        */
    JsDeclareProperty(edges);
        /**
        * Returns a node
        * @param {int} Id - Id of a node.
        * @returns {module:snap.UnidirectedGraph.Node} Node.
        * @example
        * // import the analytics and la modules
        * var snap = require('qminer').snap;
        * // create a new UndirectedGraph object
        * var graph = new snap.UnidrectedGraph;
        * // add the first node to the graph
        * graph.addNode(1);
        * // get a node of a graph
        * var node = graph.node(1);
        */
    JsDeclareFunction(node);
    /**
        * Returns the first node of a graph
        * @returns {module:snap.UnidirectedGraph.Node} Node.
        * @example
        * // import the analytics and la modules
        * var snap = require('qminer').snap;
        * // create a new UndirectedGraph object
        * var graph = new snap.UnidrectedGraph;
        * // add the first node to the graph
        * graph.addNode(1);
        * // get a node of a graph
        * var node = graph.firstNode();
        */
        JsDeclareFunction(firstNode);
    /**
        * Returns the last node of a graph
        * @returns {module:snap.UnidirectedGraph.Node} Node.
        * @example
        * // import the analytics and la modules
        * var snap = require('qminer').snap;
        * // create a new UndirectedGraph object
        * var graph = new snap.UnidrectedGraph;
        * // add the first node to the graph
        * graph.addNode(1);
        * // get a node of a graph
        * var node = graph.lastNode();
        */
        JsDeclareFunction(lastNode);
    /**
        * Iteratses nodes of a graph
        * @returns {module:snap.UnidirectedGraph} Self.
        * @example
        * // import the snap module
        * var snap = require('qminer').snap;
        * // create a new UndirectedGraph object
        * var graph = new snap.UnidrectedGraph;
        * // add two nodes to the graph
        * graph.addNode(1);
        * graph.addNode(2);
        * // iterate nodes of the graph and print ids
        * g1.eachNode(function (N) { console.log("id: " + N.id});
        */
        JsDeclareFunction(eachNode);
        /**
        * Iterates edges of a graph
        * @returns {module:snap.UnidirectedGraph} Self.
        * @example
        * // import the snap module
        * var snap = require('qminer').snap;
        * // create a new UndirectedGraph object
        * var graph = new snap.UnidrectedGraph;
        * // add three nodes and two edges to the graph
        * graph.addNode(1);
        * graph.addNode(2);
        * graph.addNode(3);
        * graph.addEdge(1,2);
        * graph.addEdge(1,3);
        * // iterate edegs of the graph and print edges
        * g1.eachEdge(function (E) { console.log("("+E.srcId+","+E.dstId+")")});
        */
    JsDeclareFunction(eachEdge);
        /**
        * Returns graph adjecency matrix.
        * @returns {module:la.sparseMatrix} SpMat.
        * @example
        * // import the snap module
        * var snap = require('qminer').snap;
        * // create a new UndirectedGraph object
        * var graph = new snap.UnidrectedGraph;
        * // add three nodes and two edges to the graph
        * graph.addNode(1);
        * graph.addNode(2);
        * graph.addNode(3);
        * graph.addEdge(1,2);
        * graph.addEdge(1,3);
        * // returns adjecency matrix of the graph
        * mat = graph.adjMat()
        */
    JsDeclareFunction(adjMat);
        /**
        * Dumps the graph to a file or the screen.
        * @param {string} Path - Path to the file
        * @returns {module:snap.UnidirectedGraph} Self.
        * @example
        * // import the snap module
        * var snap = require('qminer').snap;
        * // create a new UndirectedGraph object
        * var graph = new snap.UnidrectedGraph;
        * // add three nodes and two edges to the graph
        * graph.addNode(1);
        * graph.addNode(2);
        * graph.addNode(3);
        * graph.addEdge(1,2);
        * graph.addEdge(1,3);
        * // returns adjecency matrix of the graph
        * graph.dump()
        */
    JsDeclareFunction(dump);
        /**
        * Returns connected components of a graph
        * @returns {module:la.sparseMatrix} SpMat.
        * @example
        * // import the snap module
        * var snap = require('qminer').snap;
        * // create a new UndirectedGraph object
        * var graph = new snap.UnidrectedGraph;
        * // add three nodes and two edges to the graph
        * graph.addNode(1);
        * graph.addNode(2);
        * graph.addNode(3);
        * graph.addEdge(1,2);
        * graph.addEdge(1,3);
        * // returns connected components of a graph in for of sparse matrix
        * ccm = graph.components()
        */
    JsDeclareFunction(components);
        /**
        * Returns clustering coeficient of a graph
        * @returns {double} Ccf.
        * @example
        * // import the snap module
        * var snap = require('qminer').snap;
        * // create a new UndirectedGraph object
        * var graph = new snap.UnidrectedGraph;
        * // add three nodes and two edges to the graph
        * graph.addNode(1);
        * graph.addNode(2);
        * graph.addNode(3);
        * graph.addEdge(1,2);
        * graph.addEdge(1,3);
        * graph.addEdge(2,3);
        * // returns clustering coefficient of a graph
        * ccf = graph.clusteringCoefficient()
        */
        JsDeclareFunction(clusteringCoefficient)
    JsDeclareFunction(renumber);
        /**
        * Returns degree centrality of a node in graph
        * @param {inr} NodeId
        * @returns {int} DegCentrality.
        * @example
        * // import the snap module
        * var snap = require('qminer').snap;
        * // create a new UndirectedGraph object
        * var graph = new snap.UnidrectedGraph;
        * // add three nodes and two edges to the graph
        * graph.addNode(1);
        * graph.addNode(2);
        * graph.addNode(3);
        * graph.addEdge(1,2);
        * graph.addEdge(1,3);
        * graph.addEdge(2,3);
        * // returns degree centrality of the node with id 1 in the graph
        * dgc = graph.degreeCentrality(1)
        */
    JsDeclareFunction(degreeCentrality);
    JsDeclareFunction(load);
    JsDeclareFunction(save);
private:
    static v8::Persistent<v8::Function> Constructor;
};

///////////////////////////////
// NodeJs-Qminer-Node

template <class T>
class TNodeJsNode : public node::ObjectWrap {
public:
    typename T::TNodeI Node;

    static void Init(v8::Local<v8::Object> exports);
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
    JsDeclareFunction(nbrId);
    JsDeclareProperty(inDeg);
    JsDeclareProperty(outDeg);
    JsDeclareFunction(next);
    JsDeclareFunction(prev);
    JsDeclareFunction(eachNbr);
    JsDeclareFunction(eachInNbr);
    JsDeclareFunction(eachOutNbr);
    JsDeclareFunction(eachEdge);
    JsDeclareFunction(eachInEdge);
    JsDeclareFunction(eachOutEdge);
private:

private:
    static v8::Persistent<v8::Function> Constructor;
};

///////////////////////////////
// NodeJs-Qminer-Node

template <class T>
class TNodeJsEdge : public node::ObjectWrap {
public:
    typename T::TEdgeI Edge;

    static void Init(v8::Local<v8::Object> exports);
    static v8::Local<v8::Object> New(const typename T::TEdgeI Edge);

public:
    TNodeJsEdge() {}
      TNodeJsEdge(typename T::TEdgeI edge) { Edge = edge; }
public:
    //#
    //# **Functions and properties:**
    //#
    JsDeclareFunction(New);
    //#- `id = edge.srcId` -- return id of source node
    JsDeclareProperty(srcId);
    //#- `id = edge.dstId` -- return id of destination node
    JsDeclareProperty(dstId);
    //#- `edge = edge.next()` -- return next edge
    JsDeclareFunction(next);

private:

private:
    static v8::Persistent<v8::Function> Constructor;
};



///// graph implementations

template <class T>
v8::Persistent<v8::Function> TNodeJsGraph<T>::Constructor;


template <class T>
v8::Local<v8::Object> TNodeJsGraph<T>::New(){
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope HandleScope(Isolate);
    EAssertR(!Constructor.IsEmpty(), "TNodeJsGraph::New: constructor is empty. Did you call TNodeJsGraph::Init(exports); in this module's init function?");

    v8::Local<v8::Function> Cons = v8::Local<v8::Function>::New(Isolate, Constructor);
    v8::MaybeLocal<v8::Object> MaybeInstance = Cons->NewInstance(Isolate->GetCurrentContext());
    v8::Local<v8::Object> Instance;
    EAssertR(MaybeInstance.ToLocal(&Instance), "TNodeJsGraph::New: failed to create instance (empty)");


    TNodeJsGraph<T>* NodeGraph = new TNodeJsGraph<T>();
    NodeGraph->Wrap(Instance);
    return HandleScope.Escape(Instance);
};

template <class T>
void TNodeJsGraph<T>::New(const v8::FunctionCallbackInfo<v8::Value>& Args){
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    EAssertR(!Constructor.IsEmpty(), "TNodeJsGraph::New: constructor is empty. Did you call TNodeJsGraph::Init(exports); in this module's init function?");
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

        v8::Local<v8::Function> Cons = v8::Local<v8::Function>::New(Isolate, Constructor);
        v8::MaybeLocal<v8::Object> MaybeInstance = Cons->NewInstance(Isolate->GetCurrentContext(), Argc, Argv);
        v8::Local<v8::Object> Instance;
        EAssertR(MaybeInstance.ToLocal(&Instance), "TNodeJsGraph::New: failed to create instance (empty)");

        Args.GetReturnValue().Set(Instance);
    }
};

template <class T>
void TNodeJsGraph<T>::addNode(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = JS_GET_HOLDER(Args);
    TNodeJsGraph* NodeJsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(Self);

    if (Args.Length() != 1) {
        Isolate->ThrowException(v8::Exception::TypeError(
            TNodeJsUtil::ToLocal(Nan::New("Expected 1 argument."))));
    }
    else if (!Args[0]->IsNumber()) {
        Isolate->ThrowException(v8::Exception::TypeError(
            TNodeJsUtil::ToLocal(Nan::New("Expected number"))));
    }
    else {
        int id = TNodeJsUtil::GetArgInt32(Args, 0);
        if (!NodeJsGraph->Graph->IsNode(id))
            NodeJsGraph->Graph->AddNode(id);
    }
}

template <class T>
void TNodeJsGraph<T>::addEdge(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = JS_GET_HOLDER(Args);
    TNodeJsGraph* NodeJsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(Self);

    if (Args.Length() != 2) {
        Isolate->ThrowException(v8::Exception::TypeError(
            TNodeJsUtil::ToLocal(Nan::New("Expected 2 arguments."))));
    }
    else {
        int SrcId = TNodeJsUtil::GetArgInt32(Args, 0);
        int DstId = TNodeJsUtil::GetArgInt32(Args, 1);
        if (NodeJsGraph->Graph->IsNode(SrcId) && NodeJsGraph->Graph->IsNode(DstId))
            NodeJsGraph->Graph->AddEdge(SrcId, DstId);
    }
}

template <class T>
void TNodeJsGraph<T>::delNode(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = JS_GET_HOLDER(Args);
    TNodeJsGraph* NodeJsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(Self);

    if (Args.Length() != 1) {
        Isolate->ThrowException(v8::Exception::TypeError(
            TNodeJsUtil::ToLocal(Nan::New("Expected 1 argument."))));
    }
    else if (!Args[0]->IsNumber()) {
        Isolate->ThrowException(v8::Exception::TypeError(
            TNodeJsUtil::ToLocal(Nan::New("Expected number"))));
    }
    else {
        int id = TNodeJsUtil::GetArgInt32(Args, 0);
        if (NodeJsGraph->Graph->IsNode(id))
            NodeJsGraph->Graph->DelNode(id);
    }
}

template <class T>
void TNodeJsGraph<T>::delEdge(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = JS_GET_HOLDER(Args);
    TNodeJsGraph* NodeJsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(Self);

    if (Args.Length() != 2) {
        Isolate->ThrowException(v8::Exception::TypeError(
            TNodeJsUtil::ToLocal(Nan::New("Expected 2 arguments."))));
    }
    else {
        int SrcId = TNodeJsUtil::GetArgInt32(Args, 0);
        int DstId = TNodeJsUtil::GetArgInt32(Args, 1);
        if (NodeJsGraph->Graph->IsEdge(SrcId, DstId))
            NodeJsGraph->Graph->DelEdge(SrcId, DstId);
    }
}

template <class T>
void TNodeJsGraph<T>::isNode(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = JS_GET_HOLDER(Args);
    TNodeJsGraph* NodeJsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(Self);

    bool node = false;

    if (Args.Length() != 1) {
        Isolate->ThrowException(v8::Exception::TypeError(
            TNodeJsUtil::ToLocal(Nan::New("Expected 1 argument."))));
        Args.GetReturnValue().Set(Nan::New(node));
    }
    else if (!Args[0]->IsNumber()) {
        Isolate->ThrowException(v8::Exception::TypeError(
            TNodeJsUtil::ToLocal(Nan::New("Expected number"))));
        Args.GetReturnValue().Set(Nan::New(node));
    }
    else {
        int id = TNodeJsUtil::GetArgInt32(Args, 0);
        node = NodeJsGraph->Graph->IsNode(id);
        Args.GetReturnValue().Set(Nan::New(node));
    }
}

template <class T>
void TNodeJsGraph<T>::isEdge(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = JS_GET_HOLDER(Args);
    TNodeJsGraph* NodeJsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(Self);

    bool edge = false;

    if (Args.Length() != 2) {
        Isolate->ThrowException(v8::Exception::TypeError(
            TNodeJsUtil::ToLocal(Nan::New("Expected 2 arguments."))));
        Args.GetReturnValue().Set(Nan::New(edge));
    }
    else {
        int SrcId = TNodeJsUtil::GetArgInt32(Args, 0);
        int DstId = TNodeJsUtil::GetArgInt32(Args, 1);
        edge = NodeJsGraph->Graph->IsEdge(SrcId, DstId);
        Args.GetReturnValue().Set(Nan::New(edge));
    }
}

template <class T>
void TNodeJsGraph<T>::firstNode(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = JS_GET_HOLDER(Args);
    TNodeJsGraph* NodeJsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(Self);
    typename T::TNodeI ReturnNode = NodeJsGraph->Graph->BegNI();
    Args.GetReturnValue().Set(TNodeJsNode<T>::New(ReturnNode));
}

template <class T>
void TNodeJsGraph<T>::lastNode(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = JS_GET_HOLDER(Args);
    TNodeJsGraph* NodeJsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(Self);
    typename T::TNodeI ReturnNode = NodeJsGraph->Graph->EndNI()--;
    Args.GetReturnValue().Set(TNodeJsNode<T>::New(ReturnNode));
}

template <class T>
void TNodeJsGraph<T>::node(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    int id = TNodeJsUtil::GetArgInt32(Args, 0);

    v8::Local<v8::Object> Self = JS_GET_HOLDER(Args);
    TNodeJsGraph* NodeJsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(Self);
    if (NodeJsGraph->Graph->IsNode(id)) {
        typename T::TNodeI ReturnNode = NodeJsGraph->Graph->GetNI(id);
        Args.GetReturnValue().Set(TNodeJsNode<T>::New(ReturnNode));
    }
    else {
        Args.GetReturnValue().Set(Nan::Null());
    }

}

template <class T>
void TNodeJsGraph<T>::nodes(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    TNodeJsGraph* JsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(JS_GET_HOLDER(Info));
    int nodes = JsGraph->Graph->GetNodes();
    Info.GetReturnValue().Set(Nan::New(nodes));
}

template <class T>
void TNodeJsGraph<T>::edges(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    TNodeJsGraph* JsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(JS_GET_HOLDER(Info));
    int edges = JsGraph->Graph->GetEdges();
    Info.GetReturnValue().Set(Nan::New(edges));
}

template <class T>
void TNodeJsGraph<T>::eachNode(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::TryCatch TryCatch(Isolate);
    v8::HandleScope HandleScope(Isolate);
    TNodeJsGraph* JsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(JS_GET_HOLDER(Args));

    v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);
    const unsigned Argc = 2;
    int Count = 0;
    for (typename T::TNodeI NI = JsGraph->Graph->BegNI(); NI < JsGraph->Graph->EndNI(); NI++)
    {
        v8::Local<v8::Object> NodeObj = TNodeJsNode<T>::New(NI);
        v8::Local<v8::Value> ArgV[Argc] = {
            NodeObj, v8::Local<v8::Number>::New(Isolate, v8::Integer::NewFromUnsigned(Isolate, Count++))
        };
        Nan::Call(Callback, Isolate->GetCurrentContext()->Global(), Argc, ArgV);
        TNodeJsUtil::CheckJSExcept(TryCatch);
        Count++;
    }

    Args.GetReturnValue().Set(JS_GET_HOLDER(Args));
}

template <class T>
void TNodeJsGraph<T>::eachEdge(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::TryCatch TryCatch(Isolate);
    TNodeJsGraph* JsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(JS_GET_HOLDER(Args));

    v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);
    const unsigned Argc = 2;
    int Count = 0;
    for (typename T::TEdgeI EI = JsGraph->Graph->BegEI(); EI < JsGraph->Graph->EndEI(); EI++)
    {
        v8::Local<v8::Object> EdgeObj = TNodeJsEdge<T>::New(EI);
        v8::Local<v8::Value> ArgV[Argc] = {
            EdgeObj, v8::Local<v8::Number>::New(Isolate, v8::Integer::NewFromUnsigned(Isolate, Count++))
        };
        Nan::Call(Callback, Isolate->GetCurrentContext()->Global(), Argc, ArgV);
        TNodeJsUtil::CheckJSExcept(TryCatch);
        Count++;
    }

    Args.GetReturnValue().Set(JS_GET_HOLDER(Args));
}

template <class T>
void TNodeJsGraph<T>::adjMat(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    TNodeJsGraph* JsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(JS_GET_HOLDER(Args));

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

    Args.GetReturnValue().Set(TNodeJsUtil::NewInstance<TNodeJsSpMat>(new TNodeJsSpMat(Mat)));
}

template <class T>
void TNodeJsGraph<T>::dump(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    TNodeJsGraph* JsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(JS_GET_HOLDER(Args));

    Nan::Utf8String str(TNodeJsUtil::ToLocal(Nan::To<v8::String>(Args[0])));
    TStr FNm = *str;

    if (FNm != "") {
        FILE* pFile = fopen(FNm.CStr(), "w");
        JsGraph->Graph->Dump(pFile);
        fclose(pFile);
    }
    else {
        JsGraph->Graph->Dump(); // how to dump this out?
    }

    Args.GetReturnValue().Set(JS_GET_HOLDER(Args));
}

template <class T>
void TNodeJsGraph<T>::clusteringCoefficient(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    TNodeJsGraph* JsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(JS_GET_HOLDER(Args));
    double Ccf = TSnap::GetClustCf(JsGraph->Graph);
    Args.GetReturnValue().Set(Nan::New(Ccf));
}

template <class T>
void TNodeJsGraph<T>::components(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    TNodeJsGraph* JsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(JS_GET_HOLDER(Args));
    bool IsWeak = Nan::To<bool>(Args[0]).FromJust();

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
        Mat[i].Sort();
    }

    Args.GetReturnValue().Set(
        TNodeJsUtil::NewInstance<TNodeJsSpMat>(new TNodeJsSpMat(Mat, TLinAlgSearch::GetMaxDimIdx(Mat) + 1)));
}

template <class T>
void TNodeJsGraph<T>::renumber(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    TNodeJsGraph* JsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(JS_GET_HOLDER(Args));
    JsGraph->Graph = TSnap::ConvertGraph<TPt<T>, TPt<T> >(JsGraph->Graph, true);
}

template <class T>
void TNodeJsGraph<T>::degreeCentrality(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
        /*
    double Dgc = 0;
    if (ArgsLen == 1) {
                TNodeJsGraph* JsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(JS_GET_HOLDER(Args));
        int NodeId = TNodeJsUtil::GetArgInt32(Args, 0);
                Dgc = TSnap::GetDegreeCentr(JsGraph->Graph, NodeId);
    }
        Args.GetReturnValue().Set(Nan::New(Dgc));
        */
}


template <class T>
void TNodeJsGraph<T>::load(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1 && Args[0]->IsObject() && TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsFIn::GetClassId()),
        "Expected a FIn object as the argument.");
    TNodeJsGraph* JsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(JS_GET_HOLDER(Args));

    TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])));
    PSIn SIn = JsFIn->SIn;
    JsGraph->Graph = JsGraph->Graph->Load(*SIn);
    Args.GetReturnValue().Set(JS_GET_HOLDER(Args));
}

template <class T>
void TNodeJsGraph<T>::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1 && Args[0]->IsObject() && TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsFOut::GetClassId()),
        "Expected a FOut object as the argument.");
    TNodeJsGraph* JsGraph = ObjectWrap::Unwrap<TNodeJsGraph>(JS_GET_HOLDER(Args));

    TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])));
    EAssertR(!JsFOut->SOut.Empty(), "Output stream closed!");
    PSOut SOut = JsFOut->SOut;
    JsGraph->Graph->Save(*SOut);
    Args.GetReturnValue().Set(Args[0]);
}


///// node implementations
template <class T>
v8::Persistent<v8::Function> TNodeJsNode<T>::Constructor;

template <class T>
void TNodeJsNode<T>::Init(v8::Local<v8::Object> exports) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> context = Nan::GetCurrentContext();

    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);
    tpl->SetClassName(TNodeJsUtil::ToLocal(Nan::New("Node")));
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
    tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("id")), _id);
    tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("deg")), _deg);
    tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("inDeg")), _inDeg);
    tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("outDeg")), _outDeg);

    Constructor.Reset(Isolate, TNodeJsUtil::ToLocal(tpl->GetFunction(context)));
    Nan::Set(exports, TNodeJsUtil::ToLocal(Nan::New("Node")),
        TNodeJsUtil::ToLocal(tpl->GetFunction(context)));
}

template <class T>
v8::Local<v8::Object> TNodeJsNode<T>::New(const typename T::TNodeI node) {

    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope HandleScope(Isolate);
    EAssertR(!Constructor.IsEmpty(), "TNodeJsNode::New: constructor is empty. Did you call TNodeJsNode::Init(exports); in this module's init function?");

    v8::Local<v8::Function> Cons = v8::Local<v8::Function>::New(Isolate, Constructor);
    v8::MaybeLocal<v8::Object> MaybeInstance = Cons->NewInstance(Isolate->GetCurrentContext());
    v8::Local<v8::Object> Instance;
    EAssertR(MaybeInstance.ToLocal(&Instance), "TNodeJsNode::New: failed to create instance (empty)");

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
void TNodeJsNode<T>::id(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    TNodeJsNode* JsNode = ObjectWrap::Unwrap<TNodeJsNode>(JS_GET_HOLDER(Info));
    int id = JsNode->Node.GetId();
    Info.GetReturnValue().Set(Nan::New(id));
}

template <class T>
void TNodeJsNode<T>::deg(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    TNodeJsNode* JsNode = ObjectWrap::Unwrap<TNodeJsNode>(JS_GET_HOLDER(Info));
    int degree = JsNode->Node.GetDeg();
    Info.GetReturnValue().Set(Nan::New(degree));
}

template <class T>
void TNodeJsNode<T>::inDeg(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    TNodeJsNode* JsNode = ObjectWrap::Unwrap<TNodeJsNode>(JS_GET_HOLDER(Info));
    int degree = JsNode->Node.GetInDeg();
    Info.GetReturnValue().Set(Nan::New(degree));
}

template <class T>
void TNodeJsNode<T>::outDeg(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    TNodeJsNode* JsNode = ObjectWrap::Unwrap<TNodeJsNode>(JS_GET_HOLDER(Info));
    int degree = JsNode->Node.GetOutDeg();
    Info.GetReturnValue().Set(Nan::New(degree));
}

template <class T>
void TNodeJsNode<T>::nbrId(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    int N = TNodeJsUtil::GetArgInt32(Args, 0);
    TNodeJsNode* JsNode = ObjectWrap::Unwrap<TNodeJsNode>(JS_GET_HOLDER(Args));
    int nbrid = JsNode->Node.GetNbrNId(N);
    Args.GetReturnValue().Set(Nan::New(nbrid));
}

template <class T>
void TNodeJsNode<T>::next(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    //    int N = Args[0]->ToNumber()->Value();
    TNodeJsNode* JsNode = ObjectWrap::Unwrap<TNodeJsNode>(JS_GET_HOLDER(Args));
    JsNode->Node++;
    //return Args.GetReturnValue().Set(JsNode);
}

template <class T>
void TNodeJsNode<T>::prev(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    //    int N = Args[0]->ToNumber()->Value();
    TNodeJsNode* JsNode = ObjectWrap::Unwrap<TNodeJsNode>(JS_GET_HOLDER(Args));
    JsNode->Node--;
    //return Args.GetReturnValue().Set(JsNode);
}

template <class T>
void TNodeJsNode<T>::eachNbr(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::TryCatch TryCatch(Isolate);
    TNodeJsNode* JsNode = ObjectWrap::Unwrap<TNodeJsNode>(JS_GET_HOLDER(Args));

    v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);
    const unsigned Argc = 2;
    int Count = 0;
    v8::Local<v8::Value> ArgV;
    for (int i = 0; i < JsNode->Node.GetDeg(); i++) {
        int NbrId = JsNode->Node.GetNbrNId(i);
        v8::Local<v8::Value> ArgV[Argc] = {
            v8::Integer::New(Isolate, NbrId), v8::Local<v8::Number>::New(Isolate, v8::Integer::NewFromUnsigned(Isolate, Count++))
        };
        Nan::Call(Callback, Isolate->GetCurrentContext()->Global(), Argc, ArgV);
        TNodeJsUtil::CheckJSExcept(TryCatch);
        Count++;
    }

    Args.GetReturnValue().Set(JS_GET_HOLDER(Args));
}

template <class T>
void TNodeJsNode<T>::eachInNbr(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::TryCatch TryCatch(Isolate);
    TNodeJsNode* JsNode = ObjectWrap::Unwrap<TNodeJsNode>(JS_GET_HOLDER(Args));

    v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);
    const unsigned Argc = 2;
    int Count = 0;
    v8::Local<v8::Value> ArgV;
    for (int i = 0; i < JsNode->Node.GetInDeg(); i++) {
        int NbrId = JsNode->Node.GetInNId(i);
        v8::Local<v8::Value> ArgV[Argc] = {
            v8::Integer::New(Isolate, NbrId), v8::Local<v8::Number>::New(Isolate, v8::Integer::NewFromUnsigned(Isolate, Count++))
        };
        Nan::Call(Callback, Isolate->GetCurrentContext()->Global(), Argc, ArgV);
        TNodeJsUtil::CheckJSExcept(TryCatch);
        Count++;
    }

    Args.GetReturnValue().Set(JS_GET_HOLDER(Args));
}

template <class T>
void TNodeJsNode<T>::eachOutNbr(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::TryCatch TryCatch(Isolate);
    TNodeJsNode* JsNode = ObjectWrap::Unwrap<TNodeJsNode>(JS_GET_HOLDER(Args));

    v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);
    const unsigned Argc = 2;
    int Count = 0;
    v8::Local<v8::Value> ArgV;
    for (int i = 0; i < JsNode->Node.GetOutDeg(); i++) {
        int NbrId = JsNode->Node.GetOutNId(i);
        v8::Local<v8::Value> ArgV[Argc] = {
            v8::Integer::New(Isolate, NbrId), v8::Local<v8::Number>::New(Isolate, v8::Integer::NewFromUnsigned(Isolate, Count++))
        };
        Nan::Call(Callback, Isolate->GetCurrentContext()->Global(), Argc, ArgV);
        TNodeJsUtil::CheckJSExcept(TryCatch);
        Count++;
    }

    Args.GetReturnValue().Set(JS_GET_HOLDER(Args));
}

template <class T>
void TNodeJsNode<T>::eachEdge(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    Args.GetReturnValue().Set(JS_GET_HOLDER(Args));
}

template <class T>
void TNodeJsNode<T>::eachInEdge(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    Args.GetReturnValue().Set(JS_GET_HOLDER(Args));
}


template <class T>
void TNodeJsNode<T>::eachOutEdge(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    Args.GetReturnValue().Set(JS_GET_HOLDER(Args));
}


//// edge implementations

template <class T>
v8::Persistent<v8::Function> TNodeJsEdge<T>::Constructor;

template <class T>
void TNodeJsEdge<T>::Init(v8::Local<v8::Object> exports) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> context = Nan::GetCurrentContext();

    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);
    tpl->SetClassName(TNodeJsUtil::ToLocal(Nan::New("Edge")));
    // ObjectWrap uses the first internal field to store the wrapped pointer.
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("srcId")), _srcId);
    tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("dstId")), _dstId);
    NODE_SET_PROTOTYPE_METHOD(tpl, "next", _next);

    Constructor.Reset(Isolate, TNodeJsUtil::ToLocal(tpl->GetFunction(context)));
    Nan::Set(exports, TNodeJsUtil::ToLocal(Nan::New("Edge")),
        TNodeJsUtil::ToLocal(tpl->GetFunction(context)));
}


template <class T>
v8::Local<v8::Object> TNodeJsEdge<T>::New(const typename T::TEdgeI edge) {

    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope HandleScope(Isolate);
    EAssertR(!Constructor.IsEmpty(), "TNodeJsEdge::New: constructor is empty. Did you call TNodeEdge::Init(exports); in this module's init function?");
    v8::Local<v8::Function> Cons = v8::Local<v8::Function>::New(Isolate, Constructor);
    v8::MaybeLocal<v8::Object> MaybeInstance = Cons->NewInstance(Isolate->GetCurrentContext());
    v8::Local<v8::Object> Instance;
    EAssertR(MaybeInstance.ToLocal(&Instance), "TNodeJsEdge::New: failed to create instance (empty)");

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
void TNodeJsEdge<T>::srcId(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    TNodeJsEdge* JsEdge = ObjectWrap::Unwrap<TNodeJsEdge>(JS_GET_HOLDER(Info));
    int id = JsEdge->Edge.GetSrcNId();
    Info.GetReturnValue().Set(Nan::New(id));
}

template <class T>
void TNodeJsEdge<T>::dstId(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    TNodeJsEdge* JsEdge = ObjectWrap::Unwrap<TNodeJsEdge>(JS_GET_HOLDER(Info));
    int id = JsEdge->Edge.GetDstNId();
    Info.GetReturnValue().Set(Nan::New(id));
}

template <class T>
void TNodeJsEdge<T>::next(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    TNodeJsEdge* JsEdge = ObjectWrap::Unwrap<TNodeJsEdge>(JS_GET_HOLDER(Args));
    JsEdge->Edge++;
    Args.GetReturnValue().Set(JS_GET_HOLDER(Args));
}


#endif



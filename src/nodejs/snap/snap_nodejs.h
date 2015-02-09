#ifndef QMINER_SNAP_NODEJS
#define QMINER_SNAP_NODEJS

#ifndef BUILDING_NODE_EXTENSION
	#define BUILDING_NODE_EXTENSION
#endif

#include <node.h>
#include <node_object_wrap.h>
#include "base.h"
#include "../nodeutil.h"
#include "../fs/fs_nodejs.h"
#include "../la/la_nodejs.h"
#include "Snap.h"

///////////////////////////////
// NodeJs-Qminer-Snap
// XXX: Current implementation doesn't support all documented properties! 

class TNodeJsSnap : public node::ObjectWrap {
public:
	static void Init(v8::Handle<v8::Object> exports);
	//# 
	//# **Functions and properties:**
	//# 
	//#- `graph = snap.newUGraph()` -- generate an empty undirected graph
private:
	JsDeclareFunction(cmtyEvolution);
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
	JsDeclareFunction(eachNode);
	JsDeclareFunction(eachEdge);
	JsDeclareFunction(adjMat);
	JsDeclareFunction(dump);
	JsDeclareFunction(components);
	JsDeclareFunction(degreeCentrality);
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
	static v8::Persistent<v8::Function> constructor;
};

///////////////////////////////
// NodeJs-Qminer-Node

template <class T>
class TNodeJsEdge : public node::ObjectWrap {
public:
	typename T::TEdgeI Edge;

	static void Init(v8::Handle<v8::Object> exports);
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
	static v8::Persistent<v8::Function> constructor;
};
#endif

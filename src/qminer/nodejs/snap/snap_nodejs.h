#ifndef QMINER_SNAP_NODEJS
#define QMINER_SNAP_NODEJS

#define BUILDING_NODE_EXTENSION

#include <node.h>
#include <node_object_wrap.h>
#include "base.h"
#include "utils.h"
#include "Snap.h"


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
	JsDeclareFunction(eachNode);
	JsDeclareFunction(eachEdge);
	JsDeclareFunction(adjMat);
	JsDeclareFunction(dump);
	JsDeclareFunction(components);
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

private:

private:
	static v8::Persistent<v8::Function> constructor;
};
#endif
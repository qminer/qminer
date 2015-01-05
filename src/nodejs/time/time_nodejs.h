#ifndef QMINER_TIME_NODEJS
#define QMINER_TIME_NODEJS

#ifndef BUILDING_NODE_EXTENSION
	#define BUILDING_NODE_EXTENSION
#endif

#include <node.h>
#include <node_object_wrap.h>
#include "base.h"
#include "../nodeutil.h"


///////////////////////////////
// NodeJs-Time
//#
//# ### File system
//# 
class TNodeJsTime : public node::ObjectWrap {
private:
    TNodeJsTime() { }
public:
    static void Init(v8::Handle<v8::Object> exports);
    //# 
    //# **Functions and properties:**
    //# 

};

#endif


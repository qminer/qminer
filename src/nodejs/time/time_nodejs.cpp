#include "time_nodejs.h"


///////////////////////////////
// NodeJs-Time
void TNodeJsTime::Init(v8::Handle<v8::Object> exports) {
    // Add all prototype methods, getters and setters here.
    
}


#ifndef MODULE_INCLUDE_TIME
///////////////////////////////
// Register functions, etc.  
void init(v8::Handle<v8::Object> exports) {
    TNodeJsTime::Init(exports);
}

NODE_MODULE(time, init)
#endif


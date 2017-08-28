/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

 /*
 // This code demonstrates two minimal implementations that extend the qminer nodejs module

 // To compile the external code, run the three lines below from qminer root (make sure node-gyp is up-to-date).
 // REMARKS:
 // - Add  --msvs_version=2015 after `node-gyp configure` on windows.
 // - In case of multiple include dirs and multiple source files, each item should be enclosed within ' and the items
 //   should be separated by a space (see the example below where two external source files are provided)
 call node-gyp clean
 call node-gyp configure --msvs_version=2015 -- -DADDITIONAL_QMINER_INCLUDE_DIRS="'src/third_party/external_analytics_model'" -DADDITIONAL_QMINER_SOURCES="'src/third_party/external_analytics_model/external_analytics_model.cpp' 'src/third_party/external_analytics_model/external_analytics_model.h'"
 call node-gyp build

 // Example usage:
     var qm = require('qminer');
     var hm = new qm.external.HelloModel();
     hm.hello('John'); // should print "Hello John!"
     var vec = hm.randomVector(); // returns a wrapped C++ vector (type qm.la module.Vector)
     console.log(vec.toString()); // prints the vector

     var bc = new qm.external.BoundsChecker({lowerBound: 1, upperBound: 3});
     bc.predict(0); // below lower bound, should output -1
     bc.predict(4); // above upper bound, should output 1
     bc.predict(2); // within bounds, should output 0
 */

#ifndef EXTERNAL_ANALYTICS_MODEL_H
#define EXTERNAL_ANALYTICS_MODEL_H

#include <node.h>
#include <node_object_wrap.h>
#include "../../nodejs/nodeutil.h"
#include "../../nodejs/la/la_nodejs.h" // needed by hello model when returning linear algebra objects

namespace TNodeJsExternalQmAddon {

/////////////////////////////////////////////
// QMiner-JavaScript-Hello-Model

/**
* HelloModel
* @classdesc Hello model. Implements greeting and demonstrates returning linear algebra objects.
* @class
*/
//# exports.external.HelloModel = function(arg) { return Object.create(require('qminer').external.HelloModel.prototype); };
class TNodeJsHelloModel: public node::ObjectWrap {
    friend class TNodeJsUtil;
public:
    /// Sets up the prototype
    static void Init(v8::Handle<v8::Object> exports);
    /// Returns the class ID
    static const TStr GetClassId() { return "HelloModel"; }
    /// Counts the number of times this object was destructed
    ~TNodeJsHelloModel() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; }
private:
    /// Constructor from javascript arguments
    static TNodeJsHelloModel* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);
public:
    /**
    * Prints hello `name`
    * @param {string} name - name argument
    * @example
    * var qm = require('qminer');
    * var hm = new qm.external.HelloModel();
    * hm.hello('John'); // should print "Hello John!"
    */
    //# exports.external.HelloModel.prototype.hello = function(name) { };
    JsDeclareFunction(hello);
    // gener
    /**
    * Generates a random 2d vector - demonstrates returning linear algebra objects
    * @returns {module:la.Vector} a random 2d vector
    * @example
    * var qm = require('qminer');
    * var hm = new qm.external.HelloModel();
    * var vec = hm.randomVector(); // returns a wrapped C++ vector (type qm.la module.Vector)
    * console.log(vec.toString()); // prints the vector
    */
    //# exports.external.HelloModel.prototype.randomVector = function() { Object.create(require('qminer').la.Vector.prototype) };
    JsDeclareFunction(randomVector);
};

/////////////////////////////////////////////
// QMiner-JavaScript-Bounds-Checker

/**
* BoundsChecker
* @classdesc Bounds checker. Implements simple bounds checking.
* @class
* @param {Object} [arg] - Construction arguments
* @param {number} [arg.lowerBound] - lower bound.
* @param {number} [arg.upperBound] - upper bound.
*/
//# exports.external.BoundsChecker = function(arg) { return Object.create(require('qminer').external.BoundsChecker.prototype); };
class TNodeJsBoundsChecker: public node::ObjectWrap {
    friend class TNodeJsUtil;
public:
    /// Sets up the prototype
    static void Init(v8::Handle<v8::Object> exports);
    /// Returns the class ID
    static const TStr GetClassId() { return "BoundsChecker"; }
    /// Simple constructor
    TNodeJsBoundsChecker(const double& LB, const double& UB): LowerBound(LB), UpperBound(UB) {}
    /// Counts the number of times this object was destructed
    ~TNodeJsBoundsChecker() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; }
private:
    /// Constructor from javascript arguments
    static TNodeJsBoundsChecker* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);
public:
    /**
    * Checks if an input value lies within bounds
    * @param {number} value - Input value
    * @returns {number} -1 if the input is below the lower bound, 1 if it is above the upper bound, or 0
    * @example
    * var qm = require('qminer');
    * var bc = new qm.external.BoundsChecker({lowerBound: 1, upperBound: 3});
    * bc.predict(0); // below lower bound, should output -1
    * bc.predict(4); // above upper bound, should output 1
    * bc.predict(2); // within bounds, should output 0
    */
    //# exports.external.BoundsChecker.prototype.predict = function(value) { return 0; };
    JsDeclareFunction(predict);
private:
    TFlt LowerBound;
    TFlt UpperBound;
};

/////////////////////////////////////////////
// Initialization-And-Registration

/// Initializes all models
void InitExternalAnalyticsModel(v8::Handle<v8::Object> ExportsQm);

/// This will register the function InitExternalAnalyticsModel. It will be 
/// executed in modinit.h (InitExternalQmAddons)
INIT_EXTERN_QM_ADDON(InitExternalAnalyticsModel);

} // TNodeJsExternalQmAddon namespace

#endif /* EXTERNAL_ANALYTICS_MODEL_H */


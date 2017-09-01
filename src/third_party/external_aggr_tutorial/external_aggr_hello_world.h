/**
* Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and
* contributors
*
* All rights reserved.
*
* This source code is licensed under the FreeBSD license found in the
* LICENSE file in the root directory of this source tree.
*/

/*
 // This code demonstrates a minimal implementation of a stream aggregate that greets new records.

 // To compile the external code, run the three lines below from qminer root (make sure node-gyp is up-to-date).
 // REMARKS:
 // - Add  --msvs_version=2015 after `node-gyp configure` on windows.
 // - In case of multiple include dirs and multiple source files, each item should be enclosed within ' and the items
 //   should be separated by a space (see the example below where two external source files are provided)
 node-gyp clean
 node-gyp configure -- -DADDITIONAL_QMINER_INCLUDE_DIRS="src/third_party/external_aggr_tutorial" -DADDITIONAL_QMINER_SOURCES="'src/third_party/external_aggr_tutorial/external_aggr_hello_world.cpp' 'src/third_party/external_aggr_tutorial/external_aggr_hello_world.h'"
 node-gyp build

 // Example usage:
    var qm = require('qminer');
    var base = new qm.Base({
            mode: 'createClean',
            schema: [{ name: 'Test', fields: [{ name: 'FirstName', type: 'string' }]
            }]});
    var store = base.store('Test');
    // register hello world aggregate
    var helloAggr = store.addStreamAggr({
        type: 'helloWorld',
        fieldName: 'FirstName'
    });

    store.push({ FirstName: 'John' });
    // Should print 'Hello John!'
    store.push({ FirstName: 'Mary' });
    // Should print 'Hello Mary!'
*/

// Header guard is required
#ifndef QMINER_EXTERNAL_AGGR_HELLO_WORLD_H
#define QMINER_EXTERNAL_AGGR_HELLO_WORLD_H

#include "qminer_core.h"

namespace TQm {
namespace TStreamAggrs {

///////////////////////////////
/// Hello world aggregate
class THelloWorldAggr : public TStreamAggr {
private:
    TStr FieldNm;
protected:
    /// OnAddRec (TStreamAggr API)
    void OnAddRec(const TRec& Rec, const TWPt<TStreamAggr>& CallerAggr) {
        printf("Hello %s!\n", Rec.GetFieldStr(Rec.GetStore()->GetFieldId(FieldNm)).CStr()); }
    /// Json constructor (TStreamAggr API)
    THelloWorldAggr(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TStreamAggr(Base, ParamVal) {
        FieldNm = ParamVal->GetObjStr("fieldName"); }
public:
    /// Smart pointer Json constructor (TStreamAggr API)
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) { return new THelloWorldAggr(Base, ParamVal); }
    /// Stream aggregator static type name (TStreamAggr API)
    static TStr GetType() { return "helloWorld"; }
    /// Stream aggregator type name (TStreamAggr API)
    TStr Type() const { return GetType(); }
};

// Register aggregates
void InitExternalAggrHelloWorld_Aggr() {
    TQm::TStreamAggr::Register<TQm::TStreamAggrs::THelloWorldAggr>();
}

// The argument must match the init function name
INIT_EXTERN_AGGR(InitExternalAggrHelloWorld_Aggr);

} // TStreamAggrs namespace
} // TQm namespace

#endif

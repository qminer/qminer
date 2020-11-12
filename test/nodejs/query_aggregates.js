/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

var assert = require('../../src/nodejs/scripts/assert.js');
var qm = require('../../index.js');
var fs = qm.fs;

//////////////////////////////////////////////////////////////////////////////////////
// Store creation

var store_name = "test_store";

function TestStoreManager() {
    this.base = new qm.Base({ mode: 'createClean' });
    this.base.createStore({
        "name": store_name,
        "fields": [
            { "name": "name", "type": "string" },
            { "name": "ts", "type": "datetime" }
        ],
        "keys": [
        ]
    });

    // closes the store
    this.close = function () {
        this.base.close();
    }
}

//////////////////////////////////////////////////////////////////////////////////////
//
// describe('JSON field-type tests ', function () {
//     var records = 199;
//     var min = -2392;
//     var diff = 37;
//     it('should fill store and iterate', function () {
//         var db = new TestStoreManager("json");
//         try {
//             var now = (new Date()).getTime();
//             db.base.store(store_name).push({ name: "name1", ts: now });
//             db.base.store(store_name).push({ name: "name2", ts: now - 5 * 1000 });
//             db.base.store(store_name).push({ name: "name3", ts: now - 10 * 1000 });
//
//             var rs = db.base.search({
//                 $from: store_name
//             });
//             var res = rs.aggr([
//                 { type: "timespan", field: "ts", name:"aggr1" }
//             ]);
//             console.log(JSON.stringify(res));
//         } finally {
//             db.close();
//         }
//     })
// })



//
// //////////////////////////////////////////////////////////////////////////////////////
// // Store creation
//
// var store_name = "test_store";
//
// var base = new qm.Base({ mode: 'createClean' });
// base.createStore({
//     "name": store_name,
//     "fields": [
//         { "name": "name", "type": "string" },
//         { "name": "ts", "type": "datetime" }
//     ],
//     "keys": [
//     ]
// });
//
// //////////////////////////////////////////////////////////////////////////////////////
//
// var now = (new Date()).getTime();
// base.store(store_name).push({ name: "name1", ts: now });
// base.store(store_name).push({ name: "name2", ts: now - 5 * 1000 });
// base.store(store_name).push({ name: "name3", ts: now - 10 * 1000 });
//
// var rs = base.search({
//     $from: store_name
// });
// var res = rs.aggr([
//     { type: "timespan", field: "ts", name:"aggr1" }
// ]);
// console.log(JSON.stringify(res));
//
// base.close()
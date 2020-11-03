/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

// console.log(__filename)
var assert = require('../../src/nodejs/scripts/assert.js');     //adds assert.run function
var qm = require('../../index.js');
var fs = qm.fs;

//////////////////////////////////////////////////////////////////////////////////////
// Store creation

var store_name = "test_store";
function GetStoreTemplate(field_type) {
    var res = {
        "name": store_name,
        "fields": [
            { "name": "name", "type": "string", "primary": true },
            { "name": "val", "type": "XXXXXXX" }
        ],
        "keys": [
            { "field": "val", "type": "linear" }
        ]
    };
    res.fields[1].type = field_type;
    return res;
}

function TestStoreManager(field_type) {
    this.base = new qm.Base({ mode: 'createClean' });
    this.base.createStore(GetStoreTemplate(field_type));

    // closes the store
    this.close = function () {
        this.base.close();
    }
}

//////////////////////////////////////////////////////////////////////////////////////

function PerformTest(field_type, min, max, do_round) {
    var records = 100;
    var diff = (max - min) / records;
    describe(field_type, function () {
        it('should create store', function () {
            var db = new TestStoreManager(field_type);
            db.close();
        })
        it('should fill store and iterate', function () {
            var db = new TestStoreManager(field_type);
            try {
                var vals = [];
                // fill store
                for (var i = 0; i < records; i++) {
                    var val = min + i * diff;
                    if (do_round) {
                        val = Math.round(val);
                    }
                    db.base.store(store_name).push({ name: "name" + val, val: val });
                    vals.push(val);
                }
                var rs = db.base.store(store_name).allRecords;
                assert.equal(rs.length, records);
                for (var i = 0; i < rs.length; i++) {
                    var rec = rs[i];
                    assert.equal(rec.val, vals[i]);
                }
            } finally {
                db.close();
            }
        })
        it('should fill store and query', function () {
            var db = new TestStoreManager(field_type);
            try {
                // fill store
                for (var i = 0; i < records; i++) {
                    var val = min + i * diff;
                    if (do_round) {
                        val = Math.round(val);
                    }
                    db.base.store(store_name).push({ name: "name" + val, val: val });
                }
                assert.equal(db.base.store(store_name).allRecords.length, records);
                // make some queries
                var result = db.base.search({ $from: store_name, val: { $gt: min} });
                assert.equal(result.length, records);
                result = db.base.search({ $from: store_name, val: { $gt: max} });
                assert.equal(result.length, 0);
                result = db.base.search({ $from: store_name, val: { $lt: min} });
                assert.equal(result.length, 1);
                result = db.base.search({ $from: store_name, val: min });
                assert.equal(result.length, 1);
                result = db.base.search({ $from: store_name, val: { $lt: max} });
                assert.equal(result.length, records);
            } finally {
                db.close();
            }
        })
    });
}

describe('Int-ish field-type tests ', function () {
    PerformTest("int", -100 * 256 * 256 * 256, 100 * 256 * 256 * 256, true);
    PerformTest("int16", -100  * 256, 100  * 256, true);
    PerformTest("int64", -100 * 256 * 256 * 256 * 256 * 256 * 256, 100 * 256 * 256 * 256 * 256 * 256 * 256, true);
    PerformTest("byte", 0, 255, true);
    PerformTest("uint", 0, 256 * 256 * 256 * 256 - 1, true);
    PerformTest("uint16", 0, 256 * 256 - 1, true);
    //PerformTest("uint64", 0, 256 * 256 * 256 * 256 * 256 * 256 * 256 * 256 - 1,  true);

    PerformTest("float", -1E-38, 1E-38, false);
})

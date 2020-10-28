/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

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

describe('JSON field-type tests ', function () {
    var records = 199;
    var min = -2392;
    var diff = 37;
    it('should create store', function () {
        var db = new TestStoreManager("json");
        db.close();
    })
    it('should fill store and iterate', function () {
        var db = new TestStoreManager("json");
        try {
            var vals = [];
            // fill store
            for (var i = 0; i < records; i++) {
                var val = min + i * diff;
                var data_to_store = { name: "name" + val, val: {} };
                if (i % 2 == 0) {
                    data_to_store.val.a = val;
                } else {
                    data_to_store.val.b = val;
                    data_to_store.val.c = i;
                    data_to_store.val.d = "rec" + i;
                }

                db.base.store(store_name).push(data_to_store);
                vals.push(val);
            }
            var rs = db.base.store(store_name).allRecords;
            assert.equal(rs.length, records);
            for (var i = 0; i < rs.length; i++) {
                var rec = rs[i];
                if (i % 2 == 0) {
                    assert.equal(rec.val.a, vals[i]);
                } else {
                    assert.equal(rec.val.b, vals[i]);
                    assert.equal(rec.val.c, i);
                    assert.equal(rec.val.d, "rec" + i);
                }
            }
        } finally {
            db.close();
        }
    })
})
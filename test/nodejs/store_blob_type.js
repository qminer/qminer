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
    var records = 2;
    var min = -2392;
    var diff = 37;
    it('should create store', function () {
        var db = new TestStoreManager("blob");
        db.close();
    })
    it('should fill store and iterate', function () {
        var db = new TestStoreManager("blob");
        try {
            var vals = [];
            var src_string = "0123456789abcdefghijklmnopqrstuvwzxy";
            // fill store
            for (var i = 0; i < records; i++) {
                var val = min + i * diff;
                var data_to_store = { name: "name" + val };
                if (i % 2 == 0) {
                    data_to_store.val = new Buffer(1);
                    data_to_store.val[0] = src_string.charCodeAt(i % src_string.length);
                    vals.push(data_to_store.val[0]);
                } else {
                    data_to_store.val = new Buffer(2);
                    data_to_store.val[0] = src_string.charCodeAt(i % 18);
                    data_to_store.val[1] = src_string.charCodeAt((3 * i) % 17);
                    vals.push(data_to_store.val[0]);
                    vals.push(data_to_store.val[1]);
                }
                data_to_store.val = data_to_store.val.toString('base64'); // when storing new record, we actually parse JSON object, so here data needs to be base64 encoded
                //console.log(data_to_store.val);
                db.base.store(store_name).push(data_to_store);
            }
            var rs = db.base.store(store_name).allRecords;
            assert.equal(rs.length, records);
            //console.log(vals);

            var cnt = 0;
            for (var i = 0; i < rs.length; i++) {
                var rec = rs[i];
                //console.log(rec.val.length);
                //console.log(rec.val[0]);
                if (i % 2 == 0) {
                    assert.equal(rec.val[0], vals[cnt++]);
                } else {
                    assert.equal(rec.val[0], vals[cnt++]);
                    assert.equal(rec.val[1], vals[cnt++]);
                }
            }
        } finally {
            db.close();
        }
    })
})
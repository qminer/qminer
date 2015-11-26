/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

// console.log(__filename)
var assert = require('../../src/nodejs/scripts/assert.js');     //adds assert.run function
var qm = require('qminer');
var fs = qm.fs;

//////////////////////////////////////////////////////////////////////////////////////
// Store creation

function GetStoreTemplate(field_type) {
	var res = {
        "name": "test_store",
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

function PerformTest(field_type, min, max) {
	var records = 100;
	var diff = (max - min) / records;
    describe(field_type, function () {
        it('should create store', function () {
            var db = new TestStoreManager(field_type);
			db.close();
        })	
        it('should fill store', function () {
            var db = new TestStoreManager(field_type);
			try {
				// fill store
				for (var i = 0; i < records; i++) {
					var val = Math.round(min + i * diff);
					db.base.store("test_store").push({ name: "name" + val, val: val });
				}			
				// make some queries
				var result = db.base.search({ $from: 'test_store', val: { $gt: min} });
				assert.equal(result.length, records);
				result = db.base.search({ $from: 'test_store', val: { $gt: max} });
				assert.equal(result.length, 0);
				result = db.base.search({ $from: 'test_store', val: { $lt: min} });
				assert.equal(result.length, 1);
				result = db.base.search({ $from: 'test_store', val: { $lt: max} });
				assert.equal(result.length, records);
			} finally {
				db.close();
			}
        })	
    });
}

describe('Int-ish field-type tests ', function () {
	PerformTest("int", -100 * 256 * 256 * 256, 100 * 256 * 256 * 256);
	PerformTest("int16", -100  * 256, 100  * 256);
	PerformTest("int64", -100 * 256 * 256 * 256 * 256 * 256 * 256, 100 * 256 * 256 * 256 * 256 * 256 * 256);
	PerformTest("byte", 0, 255);
	PerformTest("uint", 0, 256 * 256 * 256 * 256);
	PerformTest("uint16", 0, 256 * 256);
	PerformTest("uint64", 0, 256 * 256 * 256 * 256 * 256 * 256 * 256 * 256);
})
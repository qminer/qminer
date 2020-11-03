/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

/**
 * This file is used for debugging from Visual Studio or some other IDE to find bugs in C++ code.
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

	var db = new TestStoreManager(field_type);
			try {
				var vals = [];
				// fill store
				for (var i = 0; i < records; i++) {
					var val = Math.round(min + i * diff);
					db.base.store(store_name).push({ name: "name" + val, val: val });
					vals.push(val);
				}
				var rs = db.base.store(store_name).allRecords;
				// assert.equal(rs.length, records);
				// for (var i = 0; i < rs.length; i++) {
					// var rec = rs[i];
					// console.log("Expecting " + vals[i]);
					// assert.equal(rec.val, vals[i]);
				// }
				// make some queries
				var result = db.base.search({ $from: store_name, val: { $gt: min} });
				assert.equal(result.length, records);
				result = db.base.search({ $from: store_name, val: { $gt: max} });
				assert.equal(result.length, 0);
				result = db.base.search({ $from: store_name, val: { $lt: min} });
				assert.equal(result.length, 1);
				result = db.base.search({ $from: store_name, val: { $lt: max} });
				assert.equal(result.length, records);
			} finally {
				db.close();
			}
}

// Uncomment this code to execute appropriate code branch

//PerformTest("int", -100 * 256 * 256 * 256, 100 * 256 * 256 * 256);
//PerformTest("int16", -100  * 256, 100  * 256);
//PerformTest("int64", -100 * 256 * 256 * 256 * 256 * 256 * 256, 100 * 256 * 256 * 256 * 256 * 256 * 256);
//PerformTest("byte", 0, 255);
//PerformTest("uint", 0, 256 * 256 * 256 * 256 - 1);
//PerformTest("uint16", 0, 256 * 256 - 1);
//PerformTest("uint64", 0, 256 * 256 * 256 * 256 * 256 * 256 * 256 * 256 - 1);

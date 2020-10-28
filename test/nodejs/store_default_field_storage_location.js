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
function GetStoreTemplate() {
    var res = {
        "name": store_name,
        "fields": [
            { "name": "name", "type": "string" },
            { "name": "val", "type": "int" }
        ],
        "keys": [ ],
        "options": {
            "storage_location": "cache"
        }
    };
    return res;
}

//////////////////////////////////////////////////////////////////////////////////////

describe('Default field-location tests ', function () {
    it('should use default location for fields', function () {
        var db = new qm.Base({ mode: 'createClean' });
        db.createStore(GetStoreTemplate());
        var store = db.store(store_name);
        var store_desc = store.toJSON();
        // here we don't see the actual storage location (at least not for now), so this code is just used to execute stuff
        // and we inspect correc behavior manually.
        assert.equal(store_desc.fields.length, 2);
        db.close();
    })
})
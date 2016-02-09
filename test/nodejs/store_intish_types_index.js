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

var store_name = "Alerts";

describe('Int-ish field-type tests ', function () {
    it('should allow query with eact value over linear index', function () {
        var base = new qm.Base({ mode: 'createClean' });
        base.createStore({
            "name": store_name,
            "fields": [
                { "name": "title", "type": "string", "store" : "cache" },
                { "name": "ts", "type": "datetime", "store": "cache" },
                { "name": "is_child", "type": "byte", "store" : "cache", "default": 0 }
            ],
            "joins": [
                { "name": "children", "type": "index", "store": store_name, "storage_location" : "cache" },
                { "name": "parent", "type": "field", "store": store_name, "inverse": "children", "storage_location" : "cache" }
            ],
            "keys": [
                { "field": "title", "type": "text" },
                { "field": "ts", "type": "linear" },
                { "field": "is_child", "type": "linear" }
            ]
        });
        var store = base.store(store_name);
            
        var id1 = store.push({ title : "test title 1", ts : (new Date()).getTime(), is_child: 0 });
        var id2 = store.push({ title : "test title 2", ts : (new Date()).getTime(), is_child: 1 });

        var query = {
            $from : store_name,
            $limit : 50,
            $sort : { ts : 0 },
            is_child : 0
        };

        var rs = base.search(query);
        assert.equal(rs.length, 1);			
        assert.equal(rs[0].$id, id1);
        
        query.is_child = 1;
        rs = base.search(query);
        assert.equal(rs.length, 1);			
        assert.equal(rs[0].$id, id2);

        base.close();
    })
})
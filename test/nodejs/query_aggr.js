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

var now = Date.parse("2016-01-01T05:20:12");
var slot0 = Date.parse("2016-01-01T05:20:00");
var slot1 = Date.parse("2016-01-01T05:25:00");
var slot2 = Date.parse("2016-01-01T05:30:00");

//////////////////////////////////////////////////////////////////////////////////////
// Store creation

var store_name = "test";

describe('Query-aggregates tests ', function () {
    var base = undefined;
    var store = undefined;

    beforeEach(function () {
        base = new qm.Base({ mode: 'createClean' });
        base.createStore({
            "name": store_name,
            "fields": [
                { "name": "title", "type": "string", "store" : "cache" },
                { "name": "src", "type": "string", "store" : "cache" },
                { "name": "ts", "type": "datetime", "store": "cache" }
            ]
        });
        store = base.store(store_name);

        store.push({title: "title1", src: "src1", ts: now});
        store.push({title: "title2", src: "src1", ts: now + 2 * 60 * 1000 });
        store.push({title: "title3", src: "src2", ts: now + 7 * 60 * 1000 });
        store.push({title: "title4", src: "src3", ts: now + 14 * 60 * 1000 });
    });
    afterEach(function () {
        base.close();
    });

    //////////////////
    it('should execute count', function () {

        var query = {
            $from : store_name
        };

        var rs = base.search(query);
        var aggrs = rs.aggr([
            { type: "count", field: "src", name: "aggr_src" },
            { type: "timeline", field: "ts", name: "aggr_ts" },
            { type: "timespan", field: "ts", name: "aggr_ts2", slot_length: 5 * 60 * 1000 } // 5 minutes
        ]);
        //console.log(JSON.stringify(aggrs));
        //console.log(JSON.stringify(aggr.aggr_src.values));
        {
            var aggr0 = aggrs[0];
            assert.equal(aggr0.type, "count");
            assert.equal(aggr0.values.length, 3);

            assert.equal(aggr0.values[0].value, "src1");
            assert.equal(aggr0.values[1].value, "src2");
            assert.equal(aggr0.values[2].value, "src3");

            assert.equal(aggr0.values[0].frequency, 2);
            assert.equal(aggr0.values[1].frequency, 1);
            assert.equal(aggr0.values[2].frequency, 1);
        }
        {
            var aggr1 = aggrs[1];
            assert.equal(aggr1.type, "timeline");
            //assert.equal(aggr1.slots.length, 3);
        }
        {
            var aggr2 = aggrs[2];
            assert.equal(aggr2.type, "timespan");
            assert.equal(aggr2.slots.length, 3);

            assert.equal(aggr2.slots[0].count, 2);
            assert.equal(aggr2.slots[1].count, 1);
            assert.equal(aggr2.slots[2].count, 1);

            assert.equal(aggr2.slots[0].slot, slot0);
            assert.equal(aggr2.slots[1].slot, slot1);
            assert.equal(aggr2.slots[2].slot, slot2);
        }
    })
})

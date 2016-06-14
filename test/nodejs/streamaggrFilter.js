/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

var assert = require('../../src/nodejs/scripts/assert.js');
var qm = require('qminer');

describe('Simple linear regression test', function () {
    var base = undefined;
    var store = undefined;
    beforeEach(function () {
        base = new qm.Base({ mode: 'createClean' });
        base.createStore({
            "name": "RecordTest",
            "fields": [
              { "name": "Bool", "type": "bool", "null": true },
              { "name": "UCh", "type": "byte", "null": true },
              { "name": "Int", "type": "int", "null": true },
              { "name": "Int16", "type": "int16", "null": true },
              { "name": "Int64", "type": "int64", "null": true },
              { "name": "UInt", "type": "uint", "null": true },
              { "name": "UInt16", "type": "uint16", "null": true },
              { "name": "UInt64", "type": "uint64", "null": true },
              { "name": "Flt", "type": "float", "null": true },
              { "name": "SFlt", "type": "sfloat", "null": true },
              { "name": "Str", "type": "string", "null": true },
              { "name": "Tm", "type": "datetime", "null": true }
            ],
            "joins": [],
            "keys": []
        });
        store = base.store('RecordTest');
    });
    afterEach(function () {
        base.close();
    });

    describe('Int filter test', function () {
        it('should test int filter creation', function () {
            var aggr = new qm.StreamAggr(base, {
                type: "timeSeriesTick",
                store: "RecordTest",
                timestamp: "Tm",
                value: "Int"
            });
            var filt = store.addStreamAggr({
                type: 'recordFilterAggr',
                aggr: aggr.name,
                filter: {
                    type: 'filterByFieldInt',
                    fieldId: store.fields.find(function (x) { return x.name === 'Int'; }).id,
                    minVal: 5,
                    maxVal: 6
                }
            });

            store.push({ Int: 5 });
            assert.equal(aggr.getFloat(), 5);
            store.push({ Int: 6 });
            assert.equal(aggr.getFloat(), 6);
            store.push({ Int: 7 }); // no update
            assert.equal(aggr.getFloat(), 6);
            store.push({ Int: 1 }); // no update
            assert.equal(aggr.getFloat(), 6);

        })
    });

});
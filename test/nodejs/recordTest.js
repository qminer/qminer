/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

// console.log(__filename)
var assert = require('assert');
var qm = require('../../index.js');

var verbose = false;

describe('Record Tests', function () {
    it('should pass', function () {
        var base = new qm.Base({ mode: 'createClean' });

        if (verbose) {
            console.log("Record", "Testing record serialization/deserilization/by value");
        }
        // only report failours
        //assert.silent = !process.isArg("-verbose");
        // name of the debug process
        //assert.consoleTitle = "Record";

        base.createStore({
            "name": "RecordTest",
            "fields": [
              { "name": "Int", "type": "int" },
              { "name": "IntV", "type": "int_v", "null": true },
              { "name": "UInt64", "type": "uint64", "null": true },
              { "name": "Str", "type": "string", "null": true },
              { "name": "StrV", "type": "string_v", "null": true },
              { "name": "Bool", "type": "bool", "null": true },
              { "name": "Flt", "type": "float", "null": true },
              { "name": "FltPr", "type": "float_pair", "null": true },
              { "name": "FltV", "type": "float_v", "null": true },
              { "name": "Tm", "type": "datetime", "null": true },
              { "name": "SpV", "type": "num_sp_v", "null": true }
            ],
            "joins": [],
            "keys": []
        });

        assert(base.store("RecordTest") != null, "Retrieve 'RecordTest' store");
        var RecordTest = base.store("RecordTest");

        // adding record
        var rec = {
            Int: 123,
            UInt64: 123,
            Str: "test",
            StrV: ["test1", "test2"],
            Bool: false,
            Flt: 1.23,
            Tm: 1433116800000
        };
        // check addition
        assert.equal(RecordTest.push(rec), 0, "RecordTest.add");
        // check getters
        var recByRef = RecordTest[0];
        assert(recByRef != null, "RecordTest[0]");
        assert.equal(recByRef.Int, 123, "recByRef.Int");
        assert.equal(recByRef.UInt64, 123, "recByRef.UInt64");
        assert.equal(recByRef.Str, "test", "recByRef.Str");
        assert.equal(recByRef.Bool, false, "recByRef.Bool");
        assert.equal(recByRef.Flt, 1.23, "recByRef.Flt");
        assert.equal(recByRef.Tm.getTime(), 1433116800000, "recByRef.Tm");

        //recByRef

        // check setters
        recByRef.Int = 124;
        assert.equal(recByRef.Int, 124, "recByRef.Int");
        recByRef.UInt64 = 124;
        assert.equal(recByRef.UInt64, 124, "recByRef.UInt64");
        recByRef.Str = "tset";
        assert.equal(recByRef.Str, "tset", "recByRef.Str");
        recByRef.Bool = true;
        assert.equal(recByRef.Bool, true, "recByRef.Bool");
        recByRef.Flt = 1.24;
        assert.equal(recByRef.Flt, 1.24, "recByRef.Flt");
        recByRef.Tm = "2015-06-01T00:00:00Z";
        assert.equal(recByRef.Tm.getTime(), 1433116800000, "recByRef.Tm");
        recByRef.Tm = 1433116800000 + 5 * 60 * 1000; // +5 min
        assert.equal(recByRef.Tm.getTime(), 1433116800000 + 5 * 60 * 1000, "recByRef.Tm");

        // check by value
        var recByVal = RecordTest.newRecord(rec);
        assert(recByVal != null, "RecordTest.newRec(rec)");
        assert.equal(recByVal.Int, 123, "recByVal.Int");
        assert.equal(recByVal.UInt64, 123, "recByVal.UInt64");
        assert.equal(recByVal.Str, "test", "recByVal.Str");
        assert(recByVal.StrV != null, "recByVal.StrV");

        if (verbose) {
            console.log("Record", JSON.stringify(recByVal.StrV));
        }
        assert.equal(recByVal.Bool, false, "recByVal.Bool");
        assert.equal(recByVal.Flt, 1.23, "recByVal.Flt");

        if (verbose) {
            console.log("Rec test end");
        }

        base.close();
        //})});
    })
});

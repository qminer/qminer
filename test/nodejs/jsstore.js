var qm = require('../../index.js');
var assert = require('../../src/nodejs/scripts/assert.js');

describe('Javascript Store Simple Array Tests', function () {
    var base = undefined;
    var arr = undefined;
    var schema = undefined;
    var store = undefined;

    beforeEach(function () {
        // setup base
        base = new qm.Base({
            mode: 'createClean',
        });

        // adding record
        var rec1 = {
            Int: 123,
            IntV: [1, 2],
            UInt64: 123,
            Str: "test",
            StrV: ["test1", "test2"],
            Bool: false,
            Flt: 1.23,
            FltPr: [1.2, 3.2],
            FltV: [1.1, 2.2],
            Tm: 1433116800000,
            SpV: new qm.la.SparseVector([[0, 1.1], [2, 2.2]])
        };

        var rec2 = {
            Int: 123,
            IntV: new qm.la.IntVector([1, 2, 3]),
            UInt64: 123,
            Str: "test",
            StrV: new qm.la.StrVector(["test1", "test2"]),
            Bool: false,
            Flt: 1.23,
            FltPr: [1.2, 3.2],
            FltV: new qm.la.Vector([1.1, 2.2, 3.3]),
            Tm: "2015-12-30T00:01:02",
            SpV: new qm.la.SparseVector([[0, 1.1], [2, 2.2]])
        };

        var rec3 = {
            Int: 123,
            IntV: new qm.la.IntVector([1, 2, 3]),
            UInt64: 123,
            Str: "test",
            StrV: new qm.la.StrVector(["test1", "test2"]),
            Bool: false,
            Flt: 1.23,
            FltPr: [1.2, 3.2],
            FltV: new qm.la.Vector([1.1, 2.2, 3.3]),
            Tm: new Date(),
            SpV: new qm.la.SparseVector([[0, 1.1], [2, 2.2]])
        };
        // simple store
        arr = [rec1, rec2, rec3];
        // schema
        schema = {
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
        };
    });
    afterEach(function () {
        base.close();
    });

    describe('Simple array implementation', function () {
        it('test adding records', function () {

            store = base.createJsStore(schema,
                new function () {
                    var that = this;
                    this.schema = schema;
                    this.GetRecords = function () {
                        return arr.length;
                    },
                    this.GetField = function (recId, fieldId) {
                        var fieldName = that.schema.fields[fieldId].name;
                        return arr[recId][fieldName];
                    },
                    this.GetFirstRecId = function () {
                        return 0;
                    },
                    this.GetLastRecId = function () {
                        return arr.length - 1;
                    }
                });

            base.addJsStoreCallback(store, "GetAllRecs", function () {
                var ids = Object.keys(arr).map(function (x) { return parseInt(x); });
                var intV = new qm.la.IntVector(ids);
                return store.newRecordSet(intV);
            });
            assert.strictEqual(store.length, 3);

            assert.strictEqual(store[0].Int, arr[0].Int);
            assert.strictEqual(store[0].IntV[0], arr[0].IntV[0]);
            assert.strictEqual(store[0].IntV[1], arr[0].IntV[1]);

            assert.strictEqual(store[0].UInt64, arr[0].UInt64);

            assert.strictEqual(store[0].Str, arr[0].Str);
            assert.strictEqual(store[0].StrV[0], arr[0].StrV[0]);
            assert.strictEqual(store[0].StrV[1], arr[0].StrV[1]);

            assert.strictEqual(store[0].Bool, arr[0].Bool);

            assert.eqtol(store[0].Flt, arr[0].Flt);
            assert.eqtol(store[0].FltPr[0], arr[0].FltPr[0]);
            assert.eqtol(store[0].FltPr[1], arr[0].FltPr[1]);
            assert.eqtol(store[0].FltV[0], arr[0].FltV[0]);
            assert.eqtol(store[0].FltV[1], arr[0].FltV[1]);

            assert.strictEqual(store[0].Tm.getTime(), arr[0].Tm);

            var temp = store[0].SpV.full().toArray();
            assert.eqtol(temp[0], 1.1);
            assert.eqtol(temp[1], 0.0);
            assert.eqtol(temp[2], 2.2);
        });
    });
});

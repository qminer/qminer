/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

var assert = require('../../src/nodejs/scripts/assert.js');
var qm = require('qminer');
var async = require('async');

function assertUpdateSequence(recField, recValArr, updatesArr, store, aggr) {
    var recJsonArr = [];
    for (var i in recValArr) {
        var recJson = {};
        recJson[recField] = recValArr[i];
        recJsonArr.push(recJson);
    }
    assert.equal(aggr.saveJson().val, 0); // should be 0 at start!
    for (var i in recJsonArr) {
        store.push(recJsonArr[i]);
        assert.equal(aggr.saveJson().val, updatesArr[i]);
    }    
}

function JsAggr() {
    var updates = 0;
    this.name = 'simple';
    this.onAdd = function (rec) { updates++; }
    this.saveJson = function (limit) { return { val: updates }; }
}


describe('Stream aggregate filter', function () {
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
            "joins": [{ name: "joinTest", type: "index", store: "RecordTest" }],
            "keys": []
        });
        store = base.store('RecordTest');
    });
    afterEach(function () {
        base.close();
    });

    describe('Constructor test', function () {
        it('should should throw exception', function () {
            var aggr = new qm.StreamAggr(base, new function () {
                var updates = 0;
                this.name = 'simple';
                this.onAdd = function (rec) {
                    updates++;
                }
                this.saveJson = function (limit) {
                    return { val: updates };
                }
            });  

            var OKInput = [{ type: "recordFilterAggr", aggr: aggr.name, filter: { type: "trivial" } }];
            OKInput.push({ type: "recordFilterAggr", aggr: aggr.name, filter: { type: "field", store: "RecordTest", field: "Int", minValue: 5 } });
            // missing fields
            var BADInput = [{ type: "recordFilterAggr", filter: { type: "field", store: "RecordTest", field: "Int", minValue: 5 } }];
            BADInput.push({ type: "recordFilterAggr", aggr: aggr.name });
            BADInput.push({ type: "recordFilterAggr", aggr: aggr.name, filter: { store: "RecordTest", field: "Int", minValue: 5 } });
            BADInput.push({ type: "recordFilterAggr", aggr: aggr.name, filter: { type: "field", field: "Int", minValue: 5 } });
            BADInput.push({ type: "recordFilterAggr", aggr: aggr.name, filter: { type: "field", store: "RecordTest", minValue: 5 } });
            BADInput.push({ type: "recordFilterAggr", aggr: aggr.name, filter: { type: "field", store: "RecordTest", field: "Int" } });
            // bad fields
            BADInput.push({ type: "recordFilterAggr", aggr: "lala", filter: { type: "field", store: "RecordTest", field: "Int", minValue: 5 } });
            BADInput.push({ type: "recordFilterAggr", aggr: aggr.name, filter: { type: "lala", store: "RecordTest", field: "Int", minValue: 5 } });
            BADInput.push({ type: "recordFilterAggr", aggr: aggr.name, filter: { type: "field", store: "lala", field: "Int", minValue: 5 } });
            BADInput.push({ type: "recordFilterAggr", aggr: aggr.name, filter: { type: "field", store: "RecordTest", field: "lala", minValue: 5 } });
            BADInput.push({ type: "recordFilterAggr", aggr: aggr.name, filter: { type: "field", store: "RecordTest", field: "Int", minValue: "lala" } });
            // null fields
            BADInput.push({ type: "recordFilterAggr", aggr: null, filter: { type: "field", store: "RecordTest", field: "Int", minValue: 5 } });
            BADInput.push({ type: "recordFilterAggr", aggr: aggr.name, filter: { type: null, store: "RecordTest", field: "Int", minValue: 5 } });
            BADInput.push({ type: "recordFilterAggr", aggr: aggr.name, filter: { type: "field", store: null, field: "Int", minValue: 5 } });
            BADInput.push({ type: "recordFilterAggr", aggr: aggr.name, filter: { type: "field", store: "RecordTest", field: null, minValue: 5 } });
            BADInput.push({ type: "recordFilterAggr", aggr: aggr.name, filter: { type: "field", store: "RecordTest", field: "Int", minValue: null } });
            
            for (key in OKInput) {
                assert.doesNotThrow(function () {
                    store.addStreamAggr(OKInput[key]);
                });
            }
            for (key in BADInput) {
                assert.throws(function () {
                    store.addStreamAggr(BADInput[key]);
                });
            }
        });
    });

    var fields = ["UCh", "Int", "Int16", "Int64", "UInt", "UInt16", "UInt64", "Flt", "SFlt", "Tm"];
    describe('Numeric field range filters', function () {
        async.each(fields, function (field, callback) {
            it('should filter ' + field + ' fields outside 5 and 6', function (done) {
                var aggr = new qm.StreamAggr(base, new JsAggr);

                var filt = store.addStreamAggr({
                    type: 'recordFilterAggr',
                    aggr: aggr.name,
                    filter: {
                        type: "field",
                        store: "RecordTest",
                        field: field,
                        minValue: 5,
                        maxValue: 6
                    }
                });
                assertUpdateSequence(field, [5, 6, 7, 1], [1, 2, 2, 2], store, aggr);
                done();
            });
            callback();
        });
        async.each(fields, function (field, callback) {
            it('should filter ' + field + ' fields below 5', function (done) {
                var aggr = new qm.StreamAggr(base, new JsAggr);

                var filt = store.addStreamAggr({
                    type: 'recordFilterAggr',
                    aggr: aggr.name,
                    filter: {
                        type: "field",
                        store: "RecordTest",
                        field: field,
                        minValue: 5
                    }
                });
                assertUpdateSequence(field, [5, 6, 7, 1], [1, 2, 3, 3], store, aggr);
                done();
            });
            callback();
        });
        async.each(fields, function (field, callback) {
            it('should filter ' + field + ' fields above 6', function (done) {
                var aggr = new qm.StreamAggr(base, new JsAggr);

                var filt = store.addStreamAggr({
                    type: 'recordFilterAggr',
                    aggr: aggr.name,
                    filter: {
                        type: "field",
                        store: "RecordTest",
                        field: field,
                        maxValue: 6
                    }
                });
                assertUpdateSequence(field, [5, 6, 7, 1], [1, 2, 2, 3], store, aggr);
                done();
            });
            callback();
        });
    });

    describe('Bool field filter', function () {
        it('should filter Bool fields that are true', function (done) {
            var aggr = new qm.StreamAggr(base, new JsAggr);

            var filt = store.addStreamAggr({
                type: 'recordFilterAggr',
                aggr: aggr.name,
                filter: {
                    type: "field",
                    store: "RecordTest",
                    field: "Bool",
                    value: true
                }
            });
            assertUpdateSequence("Bool", [true, true, false, false], [1, 2, 2, 2], store, aggr);
            done();
        });
    });

    describe('String field filter', function () {
        it('should filter string fields by value', function (done) {
            var aggr = new qm.StreamAggr(base, new JsAggr);

            var filt = store.addStreamAggr({
                type: 'recordFilterAggr',
                aggr: aggr.name,
                filter: {
                    type: "field",
                    store: "RecordTest",
                    field: "Str",
                    value: "tesi"
                }
            });
            assertUpdateSequence("Str", ["tesi", "tesi", "notTesi", "TESI"], [1, 2, 2, 2], store, aggr);
            done();
        });
    });

    describe('String range filter', function () {
        it('should filter string fields by range', function (done) {
            var aggr = new qm.StreamAggr(base, new JsAggr);

            var filt = store.addStreamAggr({
                type: 'recordFilterAggr',
                aggr: aggr.name,
                filter: {
                    type: "field",
                    store: "RecordTest",
                    field: "Str",
                    minValue: "tesia",
                    maxValue: "tesic"
                }
            });
            assertUpdateSequence("Str", ["tesia", "tesib", "tesid", "tesi"], [1, 2, 2, 2], store, aggr);
            done();
        });
    });

    describe('String set filter', function () {
        it('should filter string fields by using a set', function (done) {
            var aggr = new qm.StreamAggr(base, new JsAggr);

            var filt = store.addStreamAggr({
                type: 'recordFilterAggr',
                aggr: aggr.name,
                filter: {
                    type: "field",
                    store: "RecordTest",
                    field: "Str",
                    set: ["tesi", "test"]
                }
            });
            assertUpdateSequence("Str", ["tesi", "test", "tesid", "tesii"], [1, 2, 2, 2], store, aggr);
            done();
        });
    });

    describe('Record subsampling filter', function () {
        it('should filter every second record', function (done) {
            var aggr = new qm.StreamAggr(base, new JsAggr);

            var filt = store.addStreamAggr({
                type: 'recordFilterAggr',
                aggr: aggr.name,
                filter: {
                    type: "subsampling",
                    store: "RecordTest",
                    skip: 1
                }
            });
            assertUpdateSequence("Str", ["a", "b", "c", "d", "e", "f"], [1, 1, 2, 2, 3, 3], store, aggr);
            done();
        });
    });

    describe('Record exists filter', function () {
        it('should filter records that are not in store', function (done) {
            var aggr = new qm.StreamAggr(base, new JsAggr);

            var filt = store.addStreamAggr({
                type: 'recordFilterAggr',
                aggr: aggr.name,
                filter: {
                    type: "recordExists",
                    store: "RecordTest"
                }
            });
            assertUpdateSequence("Str", ["a", "b", "c"], [1, 2, 3], store, aggr);
            var rec = store.newRecord({ Str: "test" }); //not pushed to store
            filt.onAdd(rec); // should not pass
            assert.equal(aggr.saveJson().val, 3);
            filt.onAdd(store[0]); // should pass
            assert.equal(aggr.saveJson().val, 4);
            done();
        });
    });

    describe('Record id range filter', function () {
        it('should filter records outside id range', function (done) {
            var aggr = new qm.StreamAggr(base, new JsAggr);

            var filt = store.addStreamAggr({
                type: 'recordFilterAggr',
                aggr: aggr.name,
                filter: {
                    type: "recordId",
                    store: "RecordTest",
                    minRecId: 2,
                    maxRecId: 3
                }
            });
            assertUpdateSequence("Str", ["a", "b", "c", "d", "e"], [0, 0, 1, 2, 2], store, aggr);
            done();
        });
    });

    describe('Record id set', function () {
        it('should filter records excluded from an id set', function (done) {
            var aggr = new qm.StreamAggr(base, new JsAggr);

            var filt = store.addStreamAggr({
                type: 'recordFilterAggr',
                aggr: aggr.name,
                filter: {
                    type: "recordId",
                    store: "RecordTest",
                    recIdSet: [2,3]
                }
            });
            assertUpdateSequence("Str", ["a", "b", "c", "d", "e"], [0, 0, 1, 2, 2], store, aggr);
            done();
        });
        it('should filter records contained in an id set', function (done) {
            var aggr = new qm.StreamAggr(base, new JsAggr);

            var filt = store.addStreamAggr({
                type: 'recordFilterAggr',
                aggr: aggr.name,
                filter: {
                    type: "recordId",
                    store: "RecordTest",
                    recIdSet: [2, 3],
                    inP: false
                }
            });
            assertUpdateSequence("Str", ["a", "b", "c", "d", "e"], [1, 2, 2, 2, 3], store, aggr);
            done();
        });
    });

    describe('Record fq range filter', function () {
        it.skip('should filter records with fqs out of range', function (done) {
            var aggr = new qm.StreamAggr(base, new JsAggr);

            var filt = store.addStreamAggr({
                type: 'recordFilterAggr',
                aggr: aggr.name,
                filter: {
                    type: "recordFq",
                    store: "RecordTest",
                    minFq: 5,
                    maxFq: 6
                }
            });

            store.push({ Str: "a" });
            store.push({ Str: "b" });
            store.push({ Str: "c" });
            store.push({ Str: "d" });
            store.push({ Str: "e" });
            store[0].$addJoin("joinTest", 1, 5);
            store[0].$addJoin("joinTest", 2, 6);
            store[0].$addJoin("joinTest", 3, 7);
            store[0].$addJoin("joinTest", 4, 1);

            assert.equal(aggr.saveJson().val, 0);
            filt.onAdd(store[0].joinTest[0]);
            assert.equal(aggr.saveJson().val, 1);
            filt.onAdd(store[0].joinTest[1]);
            assert.equal(aggr.saveJson().val, 2);
            filt.onAdd(store[0].joinTest[2]);
            assert.equal(aggr.saveJson().val, 2);
            filt.onAdd(store[0].joinTest[3]);
            assert.equal(aggr.saveJson().val, 2);
            done();
        });
    });

    describe('Record index join record id range filter', function () {
        it('should filter records by joined records id range', function (done) {
            var aggr = new qm.StreamAggr(base, new JsAggr);
            // at least one record in index join must have Id between minVal and maxVal
            var filt = store.addStreamAggr({
                type: 'recordFilterAggr',
                aggr: aggr.name,
                filter: {
                    type: "indexJoin",
                    store: "RecordTest",
                    join: "joinTest",
                    minRecId: 2,
                    maxRecId: 3
                }
            });

            store.push({ Str: "a" }, false);
            store.push({ Str: "b" }, false);
            store.push({ Str: "c" }, false);
            store.push({ Str: "d" }, false);
            store.push({ Str: "e" }, false);
            // store[0] OK
            store[0].$addJoin("joinTest", 1);
            store[0].$addJoin("joinTest", 2);
            store[0].$addJoin("joinTest", 3);
            store[0].$addJoin("joinTest", 4);
            // store[1] NOT OK
            store[1].$addJoin("joinTest", 0);
            store[1].$addJoin("joinTest", 1);
            // store[2] OK
            store[2].$addJoin("joinTest", 3);
            // store[3] NOT OK
            store[3].$addJoin("joinTest", 4);

            assert.equal(aggr.saveJson().val, 0);
            filt.onAdd(store[0]);
            assert.equal(aggr.saveJson().val, 1);
            filt.onAdd(store[1]);
            assert.equal(aggr.saveJson().val, 1);
            filt.onAdd(store[2]);
            assert.equal(aggr.saveJson().val, 2);
            filt.onAdd(store[3]);
            assert.equal(aggr.saveJson().val, 2);
            filt.onAdd(store[4]);
            assert.equal(aggr.saveJson().val, 2);

            done();
        });
    });

    describe('Null field tests', function () {
        it('should filter out null fields', function (done) {
            var aggr = new qm.StreamAggr(base, new JsAggr);

            var filt = store.addStreamAggr({
                type: 'recordFilterAggr',
                aggr: aggr.name,
                filter: {
                    type: "field",
                    store: "RecordTest",
                    field: "Str",
                    value: "tesi",
                    filterNull: true
                }
            });
            assertUpdateSequence("Str", ["tesi", "tesi", null, "tesii", null, "tesi", "TESI"], [1, 2, 2, 2, 2, 3, 3], store, aggr);
            done();
        });
        it('should let null fields through', function (done) {
            var aggr = new qm.StreamAggr(base, new JsAggr);

            var filt = store.addStreamAggr({
                type: 'recordFilterAggr',
                aggr: aggr.name,
                filter: {
                    type: "field",
                    store: "RecordTest",
                    field: "Str",
                    value: "tesi",
                    filterNull: false
                }
            });
            assertUpdateSequence("Str", ["tesi", "tesi", null, "tesii", null, "tesi", "TESI"], [1, 2, 3, 3, 4, 5, 5], store, aggr);
            done();
        });
    });


});

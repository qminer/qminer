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
            "joins": [],
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



});

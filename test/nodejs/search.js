/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

// console.log(__filename)
var assert = require('../../src/nodejs/scripts/assert.js'); //adds assert.run function
var qm = require('qminer');

describe('BTree Integer Search Tests', function () {
    var base = undefined;
    var store = undefined;

    beforeEach(function () {
        qm.delLock();
        base = new qm.Base({mode: 'createClean'});
		// prepare test set
        base.createStore({
            'name': 'BTreeSearchTest',
            'fields': [
              { 'name': 'Value', 'type': 'int' },
              { 'name': 'ForSort', 'type': 'float' }
            ],
            'joins': [],
            'keys': [
                { field: 'Value', type: 'linear' }
            ]
        });
        store = base.store('BTreeSearchTest');
        for (var i = 0; i < 100; i++) {
            store.push({ Value: i % 10, ForSort: Math.random() });
        }
    });
    afterEach(function () {
        base.close();
    });

    describe('Tests of base.search after initial push', function () {
        it('returns all elements >= 5', function () {
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $gt: 5}});
            assert.equal(result.length, 50);
            result.each(function (rec) { assert.equal(rec.Value >= 5, true); });
        })
        it('returns all elements <= 6', function () {
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $lt: 6}});
            assert.equal(result.length, 70);
            result.each(function (rec) { assert.equal(rec.Value <= 6, true); });
        })
        it('returns all elements >= 5 and <= 6', function () {
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $gt: 5, $lt: 6}});
            assert.equal(result.length, 20);
            result.each(function (rec) { assert.equal(rec.Value >= 5, true); });
            result.each(function (rec) { assert.equal(rec.Value <= 6, true); });
        })
        it('returns all elements >= 5 and <= 6 and sort according to ForSort field', function () {
            var result = base.search({ $from: 'BTreeSearchTest', $sort: {ForSort: 1}, Value: { $gt: 5, $lt: 6}});
            assert.equal(result.length, 20);
            result.each(function (rec) { assert.equal(rec.Value >= 5, true); });
            result.each(function (rec) { assert.equal(rec.Value <= 6, true); });
            for (var i = 1; i < result.length; i++) {
                assert.equal(result[i-1].ForSort < result[i].ForSort, true);
            }
        })
        it('returns all elements == 5', function () {
            var result = base.search({ $from: 'BTreeSearchTest', Value: 5});
            assert.equal(result.length, 10);
            result.each(function (rec) { assert.equal(rec.Value == 5, true); });
        })
        it('returns all elements', function () {
            var result = base.search({ $from: 'BTreeSearchTest', Value: { }});
            assert.equal(result.length, 100);
        })
        it('returns no elements', function () {
            var result = base.search({ $from: 'BTreeSearchTest', Value: -5});
            assert.equal(result.length, 0);
        })
    });

    describe('Tests after serialization and deserialization', function () {
        it('returns all elements >= 5', function () {
            base.close();
            base = new qm.Base({ mode: 'open'});
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $gt: 5}});
            assert.equal(result.length, 50);
            result.each(function (rec) { assert.equal(rec.Value >= 5, true); });
        })
        it('returns all elements <= 6', function () {
            base.close();
            base = new qm.Base({ mode: 'open'});
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $lt: 6}});
            assert.equal(result.length, 70);
            result.each(function (rec) { assert.equal(rec.Value <= 6, true); });
        })
        it('returns all elements >= 5 and <= 6', function () {
            base.close();
            base = new qm.Base({ mode: 'open'});
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $gt: 5, $lt: 6}});
            assert.equal(result.length, 20);
            result.each(function (rec) { assert.equal(rec.Value >= 5, true); });
            result.each(function (rec) { assert.equal(rec.Value <= 6, true); });
        })
        it('returns all elements == 5', function () {
            base.close();
            base = new qm.Base({ mode: 'open'});
            var result = base.search({ $from: 'BTreeSearchTest', Value: 5});
            assert.equal(result.length, 10);
            result.each(function (rec) { assert.equal(rec.Value == 5, true); });
        })
        it('returns all elements', function () {
            base.close();
            base = new qm.Base({ mode: 'open'});
            var result = base.search({ $from: 'BTreeSearchTest', Value: { }});
            assert.equal(result.length, 100);
        })
        it('returns no elements', function () {
            base.close();
            base = new qm.Base({ mode: 'open'});
            var result = base.search({ $from: 'BTreeSearchTest', Value: -5});
            assert.equal(result.length, 0);
        })
    });

    describe('Tests of base.search after deleting 50 records', function () {
        it('returns all elements >= 5', function () {
            store.clear(50);
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $gt: 5}});
            assert.equal(result.length, 25);
            result.each(function (rec) { assert.equal(rec.Value >= 5, true); });
        })
        it('returns all elements <= 6', function () {
            store.clear(50);
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $lt: 6}});
            assert.equal(result.length, 35);
            result.each(function (rec) { assert.equal(rec.Value <= 6, true); });
        })
        it('returns all elements >= 5 and <= 6', function () {
            store.clear(50);
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $gt: 5, $lt: 6}});
            assert.equal(result.length, 10);
            result.each(function (rec) { assert.equal(rec.Value >= 5, true); });
            result.each(function (rec) { assert.equal(rec.Value <= 6, true); });
        })
        it('returns all elements == 5', function () {
            store.clear(50);
            var result = base.search({ $from: 'BTreeSearchTest', Value: 5});
            assert.equal(result.length, 5);
            result.each(function (rec) { assert.equal(rec.Value == 5, true); });
        })
        it('returns all elements', function () {
            store.clear(50);
            var result = base.search({ $from: 'BTreeSearchTest', Value: { }});
            assert.equal(result.length, 50);
        })
        it('returns no elements', function () {
            store.clear(50);
            var result = base.search({ $from: 'BTreeSearchTest', Value: -5});
            assert.equal(result.length, 0);
        })
    });

    describe('Tests of base.search after changing rec.Value += 1', function () {
        it('returns all elements >= 5', function () {
            store.each(function (rec) { rec.Value = rec.Value + 1; });
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $gt: 5}});
            assert.equal(result.length, 60);
            result.each(function (rec) { assert.equal(rec.Value >= 5, true); });
        })
        it('returns all elements <= 6', function () {
            store.each(function (rec) { rec.Value = rec.Value + 1; });
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $lt: 6}});
            assert.equal(result.length, 60);
            result.each(function (rec) { assert.equal(rec.Value <= 6, true); });
        })
        it('returns all elements >= 5 and <= 6', function () {
            store.each(function (rec) { rec.Value = rec.Value + 1; });
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $gt: 5, $lt: 6}});
            assert.equal(result.length, 20);
            result.each(function (rec) { assert.equal(rec.Value >= 5, true); });
            result.each(function (rec) { assert.equal(rec.Value <= 6, true); });
        })
        it('returns all elements == 5', function () {
            store.each(function (rec) { rec.Value = rec.Value + 1; });
            var result = base.search({ $from: 'BTreeSearchTest', Value: 5});
            assert.equal(result.length, 10);
            result.each(function (rec) { assert.equal(rec.Value == 5, true); });
        })
        it('returns all elements', function () {
            store.each(function (rec) { rec.Value = rec.Value + 1; });
            var result = base.search({ $from: 'BTreeSearchTest', Value: { }});
            assert.equal(result.length, 100);
        })
        it('returns no elements', function () {
            store.each(function (rec) { rec.Value = rec.Value + 1; });
            var result = base.search({ $from: 'BTreeSearchTest', Value: 0});
            assert.equal(result.length, 0);
        })
    });
});

describe('BTree UInt64 Search Tests', function () {
    var base = undefined;
    var store = undefined;

    beforeEach(function () {
        qm.delLock();
        base = new qm.Base({mode: 'createClean'});
		// prepare test set
        base.createStore({
            'name': 'BTreeSearchTest',
            'fields': [
              { 'name': 'Value', 'type': 'uint64' },
              { 'name': 'ForSort', 'type': 'float' }
            ],
            'joins': [],
            'keys': [
                { field: 'Value', type: 'linear' }
            ]
        });
        store = base.store('BTreeSearchTest');
        for (var i = 0; i < 100; i++) {
            store.push({ Value: i % 10, ForSort: Math.random() });
        }
    });
    afterEach(function () {
        base.close();
    });

    describe('Tests of base.search after initial push', function () {
        it('returns all elements >= 5', function () {
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $gt: 5}});
            assert.equal(result.length, 50);
            result.each(function (rec) { assert.equal(rec.Value >= 5, true); });
        })
        it('returns all elements <= 6', function () {
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $lt: 6}});
            assert.equal(result.length, 70);
            result.each(function (rec) { assert.equal(rec.Value <= 6, true); });
        })
        it('returns all elements >= 5 and <= 6', function () {
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $gt: 5, $lt: 6}});
            assert.equal(result.length, 20);
            result.each(function (rec) { assert.equal(rec.Value >= 5, true); });
            result.each(function (rec) { assert.equal(rec.Value <= 6, true); });
        })
        it('returns all elements >= 5 and <= 6 and sort according to ForSort field', function () {
            var result = base.search({ $from: 'BTreeSearchTest', $sort: {ForSort: 1}, Value: { $gt: 5, $lt: 6}});
            assert.equal(result.length, 20);
            result.each(function (rec) { assert.equal(rec.Value >= 5, true); });
            result.each(function (rec) { assert.equal(rec.Value <= 6, true); });
            for (var i = 1; i < result.length; i++) {
                assert.equal(result[i-1].ForSort < result[i].ForSort, true);
            }
        })
        it('returns all elements == 5', function () {
            var result = base.search({ $from: 'BTreeSearchTest', Value: 5});
            assert.equal(result.length, 10);
            result.each(function (rec) { assert.equal(rec.Value == 5, true); });
        })
        it('returns all elements', function () {
            var result = base.search({ $from: 'BTreeSearchTest', Value: { }});
            assert.equal(result.length, 100);
        })
        it('returns no elements', function () {
            var result = base.search({ $from: 'BTreeSearchTest', Value: -5});
            assert.equal(result.length, 0);
        })
    });

    describe('Tests after serialization and deserialization', function () {
        it('returns all elements >= 5', function () {
            base.close();
            base = new qm.Base({ mode: 'open'});
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $gt: 5}});
            assert.equal(result.length, 50);
            result.each(function (rec) { assert.equal(rec.Value >= 5, true); });
        })
        it('returns all elements <= 6', function () {
            base.close();
            base = new qm.Base({ mode: 'open'});
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $lt: 6}});
            assert.equal(result.length, 70);
            result.each(function (rec) { assert.equal(rec.Value <= 6, true); });
        })
        it('returns all elements >= 5 and <= 6', function () {
            base.close();
            base = new qm.Base({ mode: 'open'});
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $gt: 5, $lt: 6}});
            assert.equal(result.length, 20);
            result.each(function (rec) { assert.equal(rec.Value >= 5, true); });
            result.each(function (rec) { assert.equal(rec.Value <= 6, true); });
        })
        it('returns all elements == 5', function () {
            base.close();
            base = new qm.Base({ mode: 'open'});
            var result = base.search({ $from: 'BTreeSearchTest', Value: 5});
            assert.equal(result.length, 10);
            result.each(function (rec) { assert.equal(rec.Value == 5, true); });
        })
        it('returns all elements', function () {
            base.close();
            base = new qm.Base({ mode: 'open'});
            var result = base.search({ $from: 'BTreeSearchTest', Value: { }});
            assert.equal(result.length, 100);
        })
        it('returns no elements', function () {
            base.close();
            base = new qm.Base({ mode: 'open'});
            var result = base.search({ $from: 'BTreeSearchTest', Value: -5});
            assert.equal(result.length, 0);
        })
    });

    describe('Tests of base.search after deleting 50 records', function () {
        it('returns all elements >= 5', function () {
            store.clear(50);
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $gt: 5}});
            assert.equal(result.length, 25);
            result.each(function (rec) { assert.equal(rec.Value >= 5, true); });
        })
        it('returns all elements <= 6', function () {
            store.clear(50);
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $lt: 6}});
            assert.equal(result.length, 35);
            result.each(function (rec) { assert.equal(rec.Value <= 6, true); });
        })
        it('returns all elements >= 5 and <= 6', function () {
            store.clear(50);
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $gt: 5, $lt: 6}});
            assert.equal(result.length, 10);
            result.each(function (rec) { assert.equal(rec.Value >= 5, true); });
            result.each(function (rec) { assert.equal(rec.Value <= 6, true); });
        })
        it('returns all elements == 5', function () {
            store.clear(50);
            var result = base.search({ $from: 'BTreeSearchTest', Value: 5});
            assert.equal(result.length, 5);
            result.each(function (rec) { assert.equal(rec.Value == 5, true); });
        })
        it('returns all elements', function () {
            store.clear(50);
            var result = base.search({ $from: 'BTreeSearchTest', Value: { }});
            assert.equal(result.length, 50);
        })
        it('returns no elements', function () {
            store.clear(50);
            var result = base.search({ $from: 'BTreeSearchTest', Value: -5});
            assert.equal(result.length, 0);
        })
    });

    describe('Tests of base.search after changing rec.Value += 1', function () {
        it('returns all elements >= 5', function () {
            store.each(function (rec) { rec.Value = rec.Value + 1; });
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $gt: 5}});
            assert.equal(result.length, 60);
            result.each(function (rec) { assert.equal(rec.Value >= 5, true); });
        })
        it('returns all elements <= 6', function () {
            store.each(function (rec) { rec.Value = rec.Value + 1; });
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $lt: 6}});
            assert.equal(result.length, 60);
            result.each(function (rec) { assert.equal(rec.Value <= 6, true); });
        })
        it('returns all elements >= 5 and <= 6', function () {
            store.each(function (rec) { rec.Value = rec.Value + 1; });
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $gt: 5, $lt: 6}});
            assert.equal(result.length, 20);
            result.each(function (rec) { assert.equal(rec.Value >= 5, true); });
            result.each(function (rec) { assert.equal(rec.Value <= 6, true); });
        })
        it('returns all elements == 5', function () {
            store.each(function (rec) { rec.Value = rec.Value + 1; });
            var result = base.search({ $from: 'BTreeSearchTest', Value: 5});
            assert.equal(result.length, 10);
            result.each(function (rec) { assert.equal(rec.Value == 5, true); });
        })
        it('returns all elements', function () {
            store.each(function (rec) { rec.Value = rec.Value + 1; });
            var result = base.search({ $from: 'BTreeSearchTest', Value: { }});
            assert.equal(result.length, 100);
        })
        it('returns no elements', function () {
            store.each(function (rec) { rec.Value = rec.Value + 1; });
            var result = base.search({ $from: 'BTreeSearchTest', Value: 0});
            assert.equal(result.length, 0);
        })
    });
});

describe('BTree Float Search Tests', function () {
    var base = undefined;
    var store = undefined;

    beforeEach(function () {
        qm.delLock();
        base = new qm.Base({mode: 'createClean'});
		// prepare test set
        base.createStore({
            'name': 'BTreeSearchTest',
            'fields': [
              { 'name': 'Value', 'type': 'float' },
              { 'name': 'ForSort', 'type': 'float' }
            ],
            'joins': [],
            'keys': [
                { field: 'Value', type: 'linear' }
            ]
        });
        store = base.store('BTreeSearchTest');
        for (var i = 0; i < 100; i++) {
            store.push({ Value: (i % 10) + 0.123, ForSort: Math.random() });
        }
    });
    afterEach(function () {
        base.close();
    });

    describe('Tests of base.search after initial push', function () {
        it('returns all elements >= 5', function () {
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $gt: 5}});
            assert.equal(result.length, 50);
            result.each(function (rec) { assert.equal(rec.Value >= 5, true); });
        })
        it('returns all elements <= 6', function () {
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $lt: 6}});
            assert.equal(result.length, 60);
            result.each(function (rec) { assert.equal(rec.Value <= 6, true); });
        })
        it('returns all elements >= 5 and <= 6', function () {
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $gt: 5, $lt: 6}});
            assert.equal(result.length, 10);
            result.each(function (rec) { assert.equal(rec.Value >= 5, true); });
            result.each(function (rec) { assert.equal(rec.Value <= 6, true); });
        })
        it('returns all elements >= 5 and <= 6 and sort according to ForSort field', function () {
            var result = base.search({ $from: 'BTreeSearchTest', $sort: {ForSort: 1}, Value: { $gt: 5, $lt: 6}});
            assert.equal(result.length, 10);
            result.each(function (rec) { assert.equal(rec.Value >= 5, true); });
            result.each(function (rec) { assert.equal(rec.Value <= 6, true); });
            for (var i = 1; i < result.length; i++) {
                assert.equal(result[i-1].ForSort < result[i].ForSort, true);
            }
        })
        it('returns all elements == 5.123', function () {
            var result = base.search({ $from: 'BTreeSearchTest', Value: 5.123});
            assert.equal(result.length, 10);
            result.each(function (rec) { assert.equal(rec.Value == 5.123, true); });
        })
        it('returns all elements', function () {
            var result = base.search({ $from: 'BTreeSearchTest', Value: { }});
            assert.equal(result.length, 100);
        })
        it('returns no elements', function () {
            var result = base.search({ $from: 'BTreeSearchTest', Value: -5});
            assert.equal(result.length, 0);
        })
    });

    describe('Tests after serialization and deserialization', function () {
        it('returns all elements >= 5', function () {
            base.close();
            base = new qm.Base({ mode: 'open'});
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $gt: 5}});
            assert.equal(result.length, 50);
            result.each(function (rec) { assert.equal(rec.Value >= 5, true); });
        })
        it('returns all elements <= 6', function () {
            base.close();
            base = new qm.Base({ mode: 'open'});
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $lt: 6}});
            assert.equal(result.length, 60);
            result.each(function (rec) { assert.equal(rec.Value <= 6, true); });
        })
        it('returns all elements >= 5 and <= 6', function () {
            base.close();
            base = new qm.Base({ mode: 'open'});
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $gt: 5, $lt: 6}});
            assert.equal(result.length, 10);
            result.each(function (rec) { assert.equal(rec.Value >= 5, true); });
            result.each(function (rec) { assert.equal(rec.Value <= 6, true); });
        })
        it('returns all elements == 5.123', function () {
            base.close();
            base = new qm.Base({ mode: 'open'});
            var result = base.search({ $from: 'BTreeSearchTest', Value: 5.123});
            assert.equal(result.length, 10);
            result.each(function (rec) { assert.equal(rec.Value == 5.123, true); });
        })
        it('returns all elements', function () {
            base.close();
            base = new qm.Base({ mode: 'open'});
            var result = base.search({ $from: 'BTreeSearchTest', Value: { }});
            assert.equal(result.length, 100);
        })
        it('returns no elements', function () {
            base.close();
            base = new qm.Base({ mode: 'open'});
            var result = base.search({ $from: 'BTreeSearchTest', Value: -5});
            assert.equal(result.length, 0);
        })
    });

    describe('Tests of base.search after deleting 50 records', function () {
        it('returns all elements >= 5', function () {
            store.clear(50);
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $gt: 5}});
            assert.equal(result.length, 25);
            result.each(function (rec) { assert.equal(rec.Value >= 5, true); });
        })
        it('returns all elements <= 6', function () {
            store.clear(50);
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $lt: 6}});
            assert.equal(result.length, 30);
            result.each(function (rec) { assert.equal(rec.Value <= 6, true); });
        })
        it('returns all elements >= 5 and <= 6', function () {
            store.clear(50);
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $gt: 5, $lt: 6}});
            assert.equal(result.length, 5);
            result.each(function (rec) { assert.equal(rec.Value >= 5, true); });
            result.each(function (rec) { assert.equal(rec.Value <= 6, true); });
        })
        it('returns all elements == 5.123', function () {
            store.clear(50);
            var result = base.search({ $from: 'BTreeSearchTest', Value: 5.123});
            assert.equal(result.length, 5);
            result.each(function (rec) { assert.equal(rec.Value == 5.123, true); });
        })
        it('returns all elements', function () {
            store.clear(50);
            var result = base.search({ $from: 'BTreeSearchTest', Value: { }});
            assert.equal(result.length, 50);
        })
        it('returns no elements', function () {
            store.clear(50);
            var result = base.search({ $from: 'BTreeSearchTest', Value: -5});
            assert.equal(result.length, 0);
        })
    });

    describe('Tests of base.search after changing rec.Value += 1', function () {
        it('returns all elements >= 5', function () {
            store.each(function (rec) { rec.Value = rec.Value + 1; });
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $gt: 5}});
            assert.equal(result.length, 60);
            result.each(function (rec) { assert.equal(rec.Value >= 5, true); });
        })
        it('returns all elements <= 6', function () {
            store.each(function (rec) { rec.Value = rec.Value + 1; });
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $lt: 6}});
            assert.equal(result.length, 50);
            result.each(function (rec) { assert.equal(rec.Value <= 6, true); });
        })
        it('returns all elements >= 5 and <= 6', function () {
            store.each(function (rec) { rec.Value = rec.Value + 1; });
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $gt: 5, $lt: 6}});
            assert.equal(result.length, 10);
            result.each(function (rec) { assert.equal(rec.Value >= 5, true); });
            result.each(function (rec) { assert.equal(rec.Value <= 6, true); });
        })
        it('returns all elements == 5.123', function () {
            store.each(function (rec) { rec.Value = rec.Value + 1; });
            var result = base.search({ $from: 'BTreeSearchTest', Value: 5.123});
            assert.equal(result.length, 10);
            result.each(function (rec) { assert.equal(rec.Value == 5.123, true); });
        })
        it('returns all elements', function () {
            store.each(function (rec) { rec.Value = rec.Value + 1; });
            var result = base.search({ $from: 'BTreeSearchTest', Value: { }});
            assert.equal(result.length, 100);
        })
        it('returns no elements', function () {
            store.each(function (rec) { rec.Value = rec.Value + 1; });
            var result = base.search({ $from: 'BTreeSearchTest', Value: 0});
            assert.equal(result.length, 0);
        })
    });
});

describe('BTree DateTime Search Tests', function () {
    var base = undefined;
    var store = undefined;

    beforeEach(function () {
        qm.delLock();
        base = new qm.Base({mode: 'createClean'});
		// prepare test set
        base.createStore({
            'name': 'BTreeSearchTest',
            'fields': [
              { 'name': 'Value', 'type': 'datetime' },
              { 'name': 'ForSort', 'type': 'float' }
            ],
            'joins': [],
            'keys': [
                { field: 'Value', type: 'linear' }
            ]
        });
        store = base.store('BTreeSearchTest');
        var date = new Date('2015-09-27T00:00:00.000Z');
        for (var i = 0; i < 100; i++) {
            var _date = new Date(date.getTime() + (i % 10)*60*60*1000);
            store.push({ Value: _date.toISOString(), ForSort: Math.random() });
        }
    });
    afterEach(function () {
        base.close();
    });

    describe('Tests of base.search after initial push', function () {
        it('returns all elements >= 2015-09-27T05:00:00.000Z', function () {
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $gt: '2015-09-27T05:00:00.000Z'}});
            assert.equal(result.length, 50);
            result.each(function (rec) { assert.equal(rec.Value.toISOString() >= '2015-09-27T05:00:00.000Z', true); });
        })
        it('returns all elements <= 2015-09-27T06:00:00.000Z', function () {
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $lt: '2015-09-27T06:00:00.000Z'}});
            assert.equal(result.length, 70);
            result.each(function (rec) { assert.equal(rec.Value.toISOString() <= '2015-09-27T06:00:00.000Z', true); });
        })
        it('returns all elements >= 2015-09-27T05:00:00.000 and <= 2015-09-27T06:00:00.000Z', function () {
            var result = base.search({
                $from: 'BTreeSearchTest',
                Value: { $gt: '2015-09-27T05:00:00.000Z', $lt: '2015-09-27T06:00:00.000Z'}});
            assert.equal(result.length, 20);
            result.each(function (rec) { assert.equal(rec.Value.toISOString() >= '2015-09-27T05:00:00.000Z', true); });
            result.each(function (rec) { assert.equal(rec.Value.toISOString() <= '2015-09-27T06:00:00.000Z', true); });
        })
        it('returns all elements >= 2015-09-27T05:00:00.000 and <= 2015-09-27T06:00:00.000Z and sort according to ForSort field', function () {
            var result = base.search({
                $from: 'BTreeSearchTest',
                $sort: {ForSort: 1},
                Value: { $gt: '2015-09-27T05:00:00.000Z', $lt: '2015-09-27T06:00:00.000Z'}});
            assert.equal(result.length, 20);
            result.each(function (rec) { assert.equal(rec.Value.toISOString() >= '2015-09-27T05:00:00.000Z', true); });
            result.each(function (rec) { assert.equal(rec.Value.toISOString() <= '2015-09-27T06:00:00.000Z', true); });
            for (var i = 1; i < result.length; i++) {
                assert.equal(result[i-1].ForSort < result[i].ForSort, true);
            }
        })
        it('returns all elements == 2015-09-27T05:00:00.000Z', function () {
            var result = base.search({ $from: 'BTreeSearchTest', Value: '2015-09-27T05:00:00.000Z'});
            assert.equal(result.length, 10);
            result.each(function (rec) { assert.equal(rec.Value.toISOString() == '2015-09-27T05:00:00.000Z', true); });
        })
        it('returns all elements', function () {
            var result = base.search({ $from: 'BTreeSearchTest', Value: { }});
            assert.equal(result.length, 100);
        })
        it('returns no elements', function () {
            var result = base.search({ $from: 'BTreeSearchTest', Value: -5});
            assert.equal(result.length, 0);
        })
    });

    describe('Tests of base.search passing DateTime as JavaScript timestamp (date.getTime())', function () {
        it('returns all elements >= 2015-09-27T05:00:00.000Z', function () {
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $gt: new Date('2015-09-27T05:00:00.000Z').getTime()}});
            assert.equal(result.length, 50);
            result.each(function (rec) { assert.equal(rec.Value.toISOString() >= '2015-09-27T05:00:00.000Z', true); });
        })
        it('returns all elements <= 2015-09-27T06:00:00.000Z', function () {
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $lt: new Date('2015-09-27T06:00:00.000Z').getTime()}});
            assert.equal(result.length, 70);
            result.each(function (rec) { assert.equal(rec.Value.toISOString() <= '2015-09-27T06:00:00.000Z', true); });
        })
        it('returns all elements >= 2015-09-27T05:00:00.000 and <= 2015-09-27T06:00:00.000Z', function () {
            var result = base.search({
                $from: 'BTreeSearchTest',
                Value: { $gt: new Date('2015-09-27T05:00:00.000Z').getTime(), $lt: new Date('2015-09-27T06:00:00.000Z').getTime()}});
            assert.equal(result.length, 20);
            result.each(function (rec) { assert.equal(rec.Value.toISOString() >= '2015-09-27T05:00:00.000Z', true); });
            result.each(function (rec) { assert.equal(rec.Value.toISOString() <= '2015-09-27T06:00:00.000Z', true); });
        })
        it('returns all elements == 2015-09-27T05:00:00.000Z', function () {
            var result = base.search({ $from: 'BTreeSearchTest', Value: new Date('2015-09-27T05:00:00.000Z').getTime()});
            assert.equal(result.length, 10);
            result.each(function (rec) { assert.equal(rec.Value.toISOString() == '2015-09-27T05:00:00.000Z', true); });
        })
    });

    describe('Tests of base.search after deleting 50 records', function () {
        it('returns all elements >= 2015-09-27T05:00:00.000Z', function () {
            store.clear(50);
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $gt: '2015-09-27T05:00:00.000Z'}});
            assert.equal(result.length, 25);
            result.each(function (rec) { assert.equal(rec.Value.toISOString() >= '2015-09-27T05:00:00.000Z', true); });
        })
        it('returns all elements <= 2015-09-27T06:00:00.000Z', function () {
            store.clear(50);
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $lt: '2015-09-27T06:00:00.000Z'}});
            assert.equal(result.length, 35);
            result.each(function (rec) { assert.equal(rec.Value.toISOString() <= '2015-09-27T06:00:00.000Z', true); });
        })
        it('returns all elements >= 2015-09-27T05:00:00.000 and <= 2015-09-27T06:00:00.000Z', function () {
            store.clear(50);
            var result = base.search({
                $from: 'BTreeSearchTest',
                Value: { $gt: '2015-09-27T05:00:00.000Z', $lt: '2015-09-27T06:00:00.000Z'}});
            assert.equal(result.length, 10);
            result.each(function (rec) { assert.equal(rec.Value.toISOString() >= '2015-09-27T05:00:00.000Z', true); });
            result.each(function (rec) { assert.equal(rec.Value.toISOString() <= '2015-09-27T06:00:00.000Z', true); });
        })
        it('returns all elements == 2015-09-27T05:00:00.000Z', function () {
            store.clear(50);
            var result = base.search({ $from: 'BTreeSearchTest', Value: '2015-09-27T05:00:00.000Z'});
            assert.equal(result.length, 5);
            result.each(function (rec) { assert.equal(rec.Value.toISOString() == '2015-09-27T05:00:00.000Z', true); });
        })
        it('returns all elements', function () {
            store.clear(50);
            var result = base.search({ $from: 'BTreeSearchTest', Value: { }});
            assert.equal(result.length, 50);
        })
        it('returns no elements', function () {
            store.clear(50);
            var result = base.search({ $from: 'BTreeSearchTest', Value: -5});
            assert.equal(result.length, 0);
        })
    });

    describe('Tests of base.search after changing rec.Value += 1 minute', function () {
        it('returns all elements >= 2015-09-27T05:00:00.000Z', function () {
            store.each(function (rec) { rec.Value = new Date(rec.Value.getTime() + 1000); });
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $gt: '2015-09-27T05:00:00.000Z'}});
            assert.equal(result.length, 50);
            result.each(function (rec) { assert.equal(rec.Value.toISOString() >= '2015-09-27T05:00:00.000Z', true); });
        })
        it('returns all elements <= 2015-09-27T06:00:00.000Z', function () {
            store.each(function (rec) { rec.Value = new Date(rec.Value.getTime() + 1000); });
            var result = base.search({ $from: 'BTreeSearchTest', Value: { $lt: '2015-09-27T06:00:00.000Z'}});
            assert.equal(result.length, 60);
            result.each(function (rec) { assert.equal(rec.Value.toISOString() <= '2015-09-27T06:00:00.000Z', true); });
        })
        it('returns all elements >= 2015-09-27T05:00:00.000 and <= 2015-09-27T06:00:00.000Z', function () {
            store.each(function (rec) { rec.Value = new Date(rec.Value.getTime() + 1000); });
            var result = base.search({
                $from: 'BTreeSearchTest',
                Value: { $gt: '2015-09-27T05:00:00.000Z', $lt: '2015-09-27T06:00:00.000Z'}});
            assert.equal(result.length, 10);
            result.each(function (rec) { assert.equal(rec.Value.toISOString() >= '2015-09-27T05:00:00.000Z', true); });
            result.each(function (rec) { assert.equal(rec.Value.toISOString() <= '2015-09-27T06:00:00.000Z', true); });
        })
        it('returns all elements == 2015-09-27T05:00:00.000Z', function () {
            store.each(function (rec) { rec.Value = new Date(rec.Value.getTime() + 1000); });
            var result = base.search({ $from: 'BTreeSearchTest', Value: '2015-09-27T05:00:01.000Z'});
            assert.equal(result.length, 10);
            result.each(function (rec) { assert.equal(rec.Value.toISOString() == '2015-09-27T05:00:01.000Z', true); });
        })
        it('returns all elements', function () {
            store.each(function (rec) { rec.Value = new Date(rec.Value.getTime() + 1000); });
            var result = base.search({ $from: 'BTreeSearchTest', Value: { }});
            assert.equal(result.length, 100);
        })
        it('returns no elements', function () {
            store.each(function (rec) { rec.Value = new Date(rec.Value.getTime() + 1000); });
            var result = base.search({ $from: 'BTreeSearchTest', Value: -5});
            assert.equal(result.length, 0);
        })
    });
});

describe('Gix Tests', function () {
    var base = undefined;

    beforeEach(function () {
        qm.delLock();
        base = new qm.Base({mode: 'createClean'});
    });
    afterEach(function () {
        base.close();
    });

    describe('Test creating stores with different gix types', function () {
        it('create stores with value index key', function () {
            var store = base.createStore({
                name: 'TestStore',
                fields: [ { 'name': 'Value', 'type': 'string' } ],
                joins: [ ],
                keys: [ { field: 'Value', type: 'value' } ]
            });
            var storeFull = base.createStore({
                name: 'TestStoreFull',
                fields: [ { 'name': 'Value', 'type': 'string' } ],
                joins: [ ],
                keys: [ { field: 'Value', type: 'value', storage: 'full' } ]
            });
            var storeSmall = base.createStore({
                name: 'TestStoreSmall',
                fields: [ { 'name': 'Value', 'type': 'string' } ],
                joins: [ ],
                keys: [ { field: 'Value', type: 'value', storage: 'small' } ]
            });
            var storeTiny = base.createStore({
                name: 'TestStoreTiny',
                fields: [ { 'name': 'Value', 'type': 'string' } ],
                joins: [ ],
                keys: [ { field: 'Value', type: 'value', storage: 'tiny' } ]
            });
        })
        it('create stores with bad parameters for value index key', function () {
            assert.throws(function() {
                var store = base.createStore({
                    name: 'TestStore',
                    fields: [ { 'name': 'Value', 'type': 'string' } ],
                    joins: [ ],
                    keys: [ { field: 'XXX', type: 'value' } ]
                });
            });
            assert.throws(function() {
                var store = base.createStore({
                    name: 'TestStore',
                    fields: [ { 'name': 'Value', 'type': 'string' } ],
                    joins: [ ],
                    keys: [ { field: 'Value', type: 'XXX' } ]
                });
            });
            assert.throws(function() {
                var storeFull = base.createStore({
                    name: 'TestStoreFull',
                    fields: [ { 'name': 'Value', 'type': 'string' } ],
                    joins: [ ],
                    keys: [ { field: 'Value', type: 'value', storage: 'XXX' } ]
                });
            });
        })
        it('create stores with index join', function () {
            base.createStore([{
                name: 'TestStore1',
                fields: [ { name: 'Value', type: 'string' } ],
                joins: [ { name: 'Join', type: 'index', store: 'TestStore2' } ],
                keys: [ ]
            }, {
                name: 'TestStore2',
                fields: [ { name: 'Value', type: 'string' } ],
                keys: [ ]
            }]);
            base.createStore([{
                name: 'TestStoreFull1',
                fields: [ { name: 'Value', type: 'string' } ],
                joins: [ { name: 'Join', type: 'index', store: 'TestStore2', storage: 'full' } ],
                keys: [ ]
            }, {
                name: 'TestStoreFull2',
                fields: [ { name: 'Value', type: 'string' } ],
                keys: [ ]
            }]);
            base.createStore([{
                name: 'TestStoreSmall1',
                fields: [ { name: 'Value', type: 'string' } ],
                joins: [ { name: 'Join', type: 'index', store: 'TestStore2', storage: 'small' } ],
                keys: [ ]
            }, {
                name: 'TestStoreSmall2',
                fields: [ { name: 'Value', type: 'string' } ],
                keys: [ ]
            }]);
            base.createStore([{
                name: 'TestStoreTiny1',
                fields: [ { name: 'Value', type: 'string' } ],
                joins: [ { name: 'Join', type: 'index', store: 'TestStore2', storage: 'tiny' } ],
                keys: [ ]
            }, {
                name: 'TestStoreTiny2',
                fields: [ { name: 'Value', type: 'string' } ],
                keys: [ ]
            }]);
        })
        it('create stores with field join', function () {
            base.createStore([{
                name: 'TestStore1',
                fields: [ { name: 'Value', type: 'string' } ],
                joins: [ { name: 'Join', type: 'field', store: 'TestStore2' } ],
                keys: [ ]
            }, {
                name: 'TestStore2',
                fields: [ { name: 'Value', type: 'string' } ],
                keys: [ ]
            }]);
            base.createStore([{
                name: 'TestStoreFull1',
                fields: [ { name: 'Value', type: 'string' } ],
                joins: [ { name: 'Join', type: 'field', store: 'TestStore2', storage: 'full' } ],
                keys: [ ]
            }, {
                name: 'TestStoreFull2',
                fields: [ { name: 'Value', type: 'string' } ],
                keys: [ ]
            }]);
            base.createStore([{
                name: 'TestStoreSmall1',
                fields: [ { name: 'Value', type: 'string' } ],
                joins: [ { name: 'Join', type: 'field', store: 'TestStore2', storage: 'small' } ],
                keys: [ ]
            }, {
                name: 'TestStoreSmall2',
                fields: [ { name: 'Value', type: 'string' } ],
                keys: [ ]
            }]);
            base.createStore([{
                name: 'TestStoreTiny1',
                fields: [ { name: 'Value', type: 'string' } ],
                joins: [ { name: 'Join', type: 'field', store: 'TestStore2', storage: 'tiny' } ],
                keys: [ ]
            }, {
                name: 'TestStoreTiny2',
                fields: [ { name: 'Value', type: 'string' } ],
                keys: [ ]
            }]);
            base.createStore([{
                name: 'TestStoreIntInt1',
                fields: [ { name: 'Value', type: 'string' } ],
                joins: [ { name: 'Join', type: 'field', store: 'TestStore2', storage: 'int-int' } ],
                keys: [ ]
            }, {
                name: 'TestStoreIntInt2',
                fields: [ { name: 'Value', type: 'string' } ],
                keys: [ ]
            }]);
            base.createStore([{
                name: 'TestStoreByteInt1',
                fields: [ { name: 'Value', type: 'string' } ],
                joins: [ { name: 'Join', type: 'field', store: 'TestStore2', storage: 'byte-int' } ],
                keys: [ ]
            }, {
                name: 'TestStoreByteInt2',
                fields: [ { name: 'Value', type: 'string' } ],
                keys: [ ]
            }]);
            base.createStore([{
                name: 'TestStoreUInt641',
                fields: [ { name: 'Value', type: 'string' } ],
                joins: [ { name: 'Join', type: 'field', store: 'TestStore2', storage: 'uint64-' } ],
                keys: [ ]
            }, {
                name: 'TestStoreUInt642',
                fields: [ { name: 'Value', type: 'string' } ],
                keys: [ ]
            }]);
        })
        it('create stores with bad parameters for join', function () {
            assert.throws(function() {
                base.createStore([{
                    name: 'TestStore1',
                    fields: [ { name: 'Value', type: 'string' } ],
                    joins: [ { name: 'Join', type: 'XXX', store: 'TestStore2' } ],
                    keys: [ ]
                }, {
                    name: 'TestStore2',
                    fields: [ { name: 'Value', type: 'string' } ],
                    keys: [ ]
                }]);
            });
            assert.throws(function() {
                base.createStore([{
                    name: 'TestStore1',
                    fields: [ { name: 'Value', type: 'string' } ],
                    joins: [ { name: 'Join', type: 'index', store: 'TestStore2', storage: 'XXX' } ],
                    keys: [ ]
                }, {
                    name: 'TestStore2',
                    fields: [ { name: 'Value', type: 'string' } ],
                    keys: [ ]
                }]);
            });
            assert.throws(function() {
                base.createStore([{
                    name: 'TestStore1',
                    fields: [ { name: 'Value', type: 'string' } ],
                    joins: [ { name: 'Join', type: 'field', store: 'TestStore2', storage: 'XXX' } ],
                    keys: [ ]
                }, {
                    name: 'TestStore2',
                    fields: [ { name: 'Value', type: 'string' } ],
                    keys: [ ]
                }]);
            });
            assert.throws(function() {
                base.createStore([{
                    name: 'TestStore1',
                    fields: [ { name: 'Value', type: 'string' } ],
                    joins: [ { name: 'Join', type: 'field', store: 'TestStore2', storage: 'int-XXX' } ],
                    keys: [ ]
                }, {
                    name: 'TestStore2',
                    fields: [ { name: 'Value', type: 'string' } ],
                    keys: [ ]
                }]);
            });
            assert.throws(function() {
                base.createStore([{
                    name: 'TestStore1',
                    fields: [ { name: 'Value', type: 'string' } ],
                    joins: [ { name: 'Join', type: 'field', store: 'TestStore2', storage: 'XXX-' } ],
                    keys: [ ]
                }, {
                    name: 'TestStore2',
                    fields: [ { name: 'Value', type: 'string' } ],
                    keys: [ ]
                }]);
            });

        })
    });

    function testGixSearch(gixType) {
        function prepareSimpleStore() {
            var store = base.createStore({
                name: 'TestStore',
                fields: [ { 'name': 'Value', 'type': 'string' } ],
                joins: [ ],
                keys: [ { field: 'Value', type: 'value', storage: 'full' } ]
            });
            store.push({ Value: "A" });
            store.push({ Value: "B" });
            store.push({ Value: "C" });
            store.push({ Value: "D" });
            store.push({ Value: "B" });
            store.push({ Value: "C" });
            store.push({ Value: "D" });
            store.push({ Value: "C" });
            store.push({ Value: "D" });
            store.push({ Value: "D" });
        }

        function prepareDualStore() {
            var store = base.createStore({
                name: 'TestStore',
                fields: [
                    { 'name': 'Value', 'type': 'string' },
                    { 'name': 'Count', 'type': 'int' }
                ],
                joins: [ ],
                keys: [
                    { field: 'Value', type: 'value', storage: gixType },
                    { field: 'Count', type: 'linear' }
                ]
            });
            store.push({ Value: "A", Count: 1 });
            store.push({ Value: "B", Count: 2 });
            store.push({ Value: "C", Count: 3 });
            store.push({ Value: "D", Count: 4 });
            store.push({ Value: "B", Count: 5 });
            store.push({ Value: "C", Count: 6 });
            store.push({ Value: "D", Count: 7 });
            store.push({ Value: "C", Count: 8 });
            store.push({ Value: "D", Count: 9 });
            store.push({ Value: "D", Count: 10 });
        }

        function prepareJoinStore1() {
            base.createStore([{
                name: 'TestStore1',
                fields: [ { name: 'Value', type: 'string' } ],
                joins: [ { name: 'Join', type: 'field', store: 'TestStore2', inverse: 'Join', storage: gixType } ],
                keys: [ { field: 'Value', type: 'value', storage: gixType } ]
            }, {
                name: 'TestStore2',
                fields: [ { name: 'Value', type: 'string', primary: true } ],
                joins: [ { name: 'Join', type: 'index', store: 'TestStore1', inverse: 'Join', storage: gixType } ],
                keys: [ { field: 'Value', type: 'value', storage: gixType } ]
            }]);
            var store = base.store("TestStore1");
            store.push({ Value: "A", Join: { Value: "x" }});
            store.push({ Value: "B", Join: { Value: "y" }});
            store.push({ Value: "C", Join: { Value: "z" }});
            store.push({ Value: "D", Join: { Value: "x" }});
            store.push({ Value: "B", Join: { Value: "y" }});
            store.push({ Value: "C", Join: { Value: "z" }});
            store.push({ Value: "D", Join: { Value: "x" }});
            store.push({ Value: "C", Join: { Value: "y" }});
            store.push({ Value: "D", Join: { Value: "z" }});
            store.push({ Value: "D", Join: { Value: "x" }});
        }

        describe('Test search for gix type "' + gixType + '"', function () {
            it('should return correct number of records for simple query', function () {
                prepareSimpleStore();
                assert.equal(base.search({ $from: "TestStore", Value: "A" }).length, 1);
                assert.equal(base.search({ $from: "TestStore", Value: "B" }).length, 2);
                assert.equal(base.search({ $from: "TestStore", Value: "C" }).length, 3);
                assert.equal(base.search({ $from: "TestStore", Value: "D" }).length, 4);
            })
            it('should return correct number of records for simple query after delete', function () {
                prepareSimpleStore();
                assert.equal(base.search({ $from: "TestStore", Value: "A" }).length, 1);
                assert.equal(base.search({ $from: "TestStore", Value: "B" }).length, 2);
                base.store("TestStore").clear(1);
                assert.equal(base.search({ $from: "TestStore", Value: "A" }).length, 0);
                base.store("TestStore").clear(1);
                assert.equal(base.search({ $from: "TestStore", Value: "B" }).length, 1);
                base.store("TestStore").clear(3);
                assert.equal(base.search({ $from: "TestStore", Value: "B" }).length, 0);
            })
            it('should return correct number of records for or query', function () {
                prepareSimpleStore();
                assert.equal(base.search({
                    $from: "TestStore",
                    $or:  [ { Value: "A" }, { Value: "B"} ]
                }).length, 3);
            })
            it('should return correct number of records for and query', function () {
                prepareDualStore();
                assert.equal(base.search({
                    $from: "TestStore",
                    Value: "D",
                    Count: { $gt: 6 }
                }).length, 3);
                assert.equal(base.search({
                    $from: "TestStore",
                    Value: "B",
                    Count: { $gt: 6 }
                }).length, 0);
            })
            it('should return correct number of records for join query', function () {
                prepareJoinStore1();
                assert.equal(base.store("TestStore1").length, 10);
                assert.equal(base.store("TestStore2").length, 3);
                // x < y < z
                assert.equal(base.store("TestStore2").recordByName("x").$id, 0);
                assert.equal(base.store("TestStore2").recordByName("y").$id, 1);
                assert.equal(base.store("TestStore2").recordByName("z").$id, 2);
                // x => [A, D, D, D]
                var res1 = base.search({
                    $join: {
                        $name: "Join",
                        $query: { $from: "TestStore2", Value: "x" }
                    }
                });
                assert.equal(res1.length, 4);
                assert.equal(res1[0].Value, "A");
                assert.equal(res1[1].Value, "D");
                assert.equal(res1[2].Value, "D");
                assert.equal(res1[3].Value, "D");
                // A => [x]
                var res2 = base.search({
                    $join: {
                        $name: "Join",
                        $query: { $from: "TestStore1", Value: "A" }
                    }
                });
                assert.equal(res2.length, 1);
                assert.equal(res2[0].Value, "x");
                //C => [y, z]
                var res3 = base.search({
                    $join: {
                        $name: "Join",
                        $query: { $from: "TestStore1", Value: "C" }
                    }
                });
                assert.equal(res3.length, 2);
                assert.equal(res3[0].Value, "y");
                assert.equal(res3[0].$fq, 1);
                assert.equal(res3[1].Value, "z");
                assert.equal(res3[1].$fq, 2);
                //A => [x] => [A, D, D, D]
                var res4 = base.search({
                    $join: {
                        $name: "Join",
                        $query: {
                            $join: {
                                $name: "Join",
                                $query: {
                                    $from: "TestStore1", Value: "A"
                                }
                            }
                        }
                    }
                });
                assert.equal(res4.length, 4);
                assert.equal(res4[0].Value, "A");
                assert.equal(res4[0].$fq, 1);
                assert.equal(res4[1].Value, "D");
                assert.equal(res4[1].$fq, 1);
                assert.equal(res4[2].Value, "D");
                assert.equal(res4[2].$fq, 1);
                assert.equal(res4[3].Value, "D");
                assert.equal(res4[3].$fq, 1);
            })
            it('should return correct number of records for join query after delete', function () {
                prepareJoinStore1();
                // x => [D, D, D]
                base.store("TestStore1").clear(1);
                var res1 = base.search({
                    $join: {
                        $name: "Join",
                        $query: { $from: "TestStore2", Value: "x" }
                    }
                });
                assert.equal(res1.length, 3);
                assert.equal(res1[0].Value, "D");
                assert.equal(res1[1].Value, "D");
                assert.equal(res1[2].Value, "D");
            })
            it('should return correct number of records for join query after delete', function () {
                prepareJoinStore1();
                // delete x
                base.store("TestStore2").clear(1);
                //C => [y, z]
                var res3 = base.search({
                    $join: {
                        $name: "Join",
                        $query: { $from: "TestStore1", Value: "C" }
                    }
                });
                assert.equal(res3.length, 2);
                assert.equal(res3[0].Value, "y");
                assert.equal(res3[1].Value, "z");
                // delete y
                base.store("TestStore2").clear(1);
                //C => [z]
                var res3 = base.search({
                    $join: {
                        $name: "Join",
                        $query: { $from: "TestStore1", Value: "C" }
                    }
                });
                assert.equal(res3.length, 1);
                assert.equal(res3[0].Value, "z");
            })
        })
    }

    testGixSearch("full");
    testGixSearch("small");
    testGixSearch("tiny");

    function testGixFrequency1(gixType) {
        function prepareJoinStore() {
            base.createStore([{
                name: 'TestStore1',
                fields: [ { name: 'Value', type: 'string', primary: true } ],
                joins: [ { name: 'Join', type: 'index', store: 'TestStore2', inverse: 'Join', storage: gixType } ],
                keys: [ { field: 'Value', type: 'value', storage: gixType } ]
            }, {
                name: 'TestStore2',
                fields: [ { name: 'Value', type: 'string', primary: true } ],
                joins: [ { name: 'Join', type: 'index', store: 'TestStore1', inverse: 'Join', storage: gixType } ],
                keys: [ { field: 'Value', type: 'value', storage: gixType } ]
            }]);
            var store1 = base.store("TestStore1");
            store1.push({ Value: "A" });
            store1.push({ Value: "B" });
            store1.push({ Value: "C" });
            store1.push({ Value: "D" });
            var store2 = base.store("TestStore2");
            store2.push({ Value: "x" });
            store2.push({ Value: "y" });
            store2.push({ Value: "z" });
            store1[0].$addJoin("Join", store2[0], 1); // A->x:1
            store1[0].$addJoin("Join", store2[1], 2); // A->y:2
            store1[0].$addJoin("Join", store2[2], 4); // A->z:4
            store1[1].$addJoin("Join", store2[0], 5); // B->x:5
            store1[1].$addJoin("Join", store2[1], 6); // B->y:6
            store1[2].$addJoin("Join", store2[2], 1); // C->z:1
            store1[3].$addJoin("Join", store2[0], 2); // D->x:2
        }
        describe('Test explicit frequencies xfor gix type "' + gixType + '"', function () {
            it('should return correct frequencies for records when doing direct join on record', function () {
                prepareJoinStore()
                // first direct joins, without search
                var rec1 = base.store("TestStore1").recordByName("A");
                assert.equal(rec1.Join.length, 3);
                assert.equal(rec1.Join[0].Value, "x");
                assert.equal(rec1.Join[0].$id, 0);
                assert.equal(rec1.Join[0].$fq, 1);
                assert.equal(rec1.Join[1].Value, "y");
                assert.equal(rec1.Join[1].$id, 1);
                assert.equal(rec1.Join[1].$fq, 2);
                assert.equal(rec1.Join[2].Value, "z");
                assert.equal(rec1.Join[2].$id, 2);
                assert.equal(rec1.Join[2].$fq, 4);

                var rec2 = base.store("TestStore1").recordByName("B");
                assert.equal(rec2.Join.length, 2);
                assert.equal(rec2.Join[0].Value, "x");
                assert.equal(rec2.Join[0].$id, 0);
                assert.equal(rec2.Join[0].$fq, 5);
                assert.equal(rec2.Join[1].Value, "y");
                assert.equal(rec2.Join[1].$id, 1);
                assert.equal(rec2.Join[1].$fq, 6);

                var rec3 = base.store("TestStore1").recordByName("C");
                assert.equal(rec3.Join.length, 1);
                assert.equal(rec3.Join[0].Value, "z");
                assert.equal(rec3.Join[0].$id, 2);
                assert.equal(rec3.Join[0].$fq, 1);

                var rec4 = base.store("TestStore1").recordByName("D");
                assert.equal(rec4.Join.length, 1);
                assert.equal(rec4.Join[0].Value, "x");
                assert.equal(rec4.Join[0].$id, 0);
                assert.equal(rec4.Join[0].$fq, 2);

                var rec5 = base.store("TestStore2").recordByName("x");
                assert.equal(rec5.Join.length, 3);
                assert.equal(rec5.Join[0].Value, "A");
                assert.equal(rec5.Join[0].$id, 0);
                assert.equal(rec5.Join[0].$fq, 1);
                assert.equal(rec5.Join[1].Value, "B");
                assert.equal(rec5.Join[1].$id, 1);
                assert.equal(rec5.Join[1].$fq, 5);
                assert.equal(rec5.Join[2].Value, "D");
                assert.equal(rec5.Join[2].$id, 3);
                assert.equal(rec5.Join[2].$fq, 2);

                var rec6 = base.store("TestStore2").recordByName("y");
                assert.equal(rec6.Join.length, 2);
                assert.equal(rec6.Join[0].Value, "A");
                assert.equal(rec6.Join[0].$id, 0);
                assert.equal(rec6.Join[0].$fq, 2);
                assert.equal(rec6.Join[1].Value, "B");
                assert.equal(rec6.Join[1].$id, 1);
                assert.equal(rec6.Join[1].$fq, 6);

                var rec7 = base.store("TestStore2").recordByName("z");
                assert.equal(rec7.Join.length, 2);
                assert.equal(rec7.Join[0].Value, "A");
                assert.equal(rec7.Join[0].$id, 0);
                assert.equal(rec7.Join[0].$fq, 4);
                assert.equal(rec7.Join[1].Value, "C");
                assert.equal(rec7.Join[1].$id, 2);
                assert.equal(rec7.Join[1].$fq, 1);
            })
            it('should return correct frequencies for records when doing direct join or recordset', function () {
                prepareJoinStore()
                // first direct joins, without search
                var rec1 = base.search({ $from: "TestStore1", Value: "A" }).join("Join");
                assert.equal(rec1.length, 3);
                assert.equal(rec1[0].Value, "x");
                assert.equal(rec1[0].$id, 0);
                assert.equal(rec1[0].$fq, 1);
                assert.equal(rec1[1].Value, "y");
                assert.equal(rec1[1].$id, 1);
                assert.equal(rec1[1].$fq, 2);
                assert.equal(rec1[2].Value, "z");
                assert.equal(rec1[2].$id, 2);
                assert.equal(rec1[2].$fq, 4);

                var rec2 = base.search({ $from: "TestStore1", Value: "B" }).join("Join");
                assert.equal(rec2.length, 2);
                assert.equal(rec2[0].Value, "x");
                assert.equal(rec2[0].$id, 0);
                assert.equal(rec2[0].$fq, 5);
                assert.equal(rec2[1].Value, "y");
                assert.equal(rec2[1].$id, 1);
                assert.equal(rec2[1].$fq, 6);

                var rec3 = base.search({ $from: "TestStore1", Value: "C" }).join("Join");
                assert.equal(rec3.length, 1);
                assert.equal(rec3[0].Value, "z");
                assert.equal(rec3[0].$id, 2);
                assert.equal(rec3[0].$fq, 1);

                var rec4 = base.search({ $from: "TestStore1", Value: "D" }).join("Join");
                assert.equal(rec4.length, 1);
                assert.equal(rec4[0].Value, "x");
                assert.equal(rec4[0].$id, 0);
                assert.equal(rec4[0].$fq, 2);

                var rec5 = base.search({ $from: "TestStore2", Value: "x" }).join("Join");
                assert.equal(rec5.length, 3);
                assert.equal(rec5[0].Value, "A");
                assert.equal(rec5[0].$id, 0);
                assert.equal(rec5[0].$fq, 1);
                assert.equal(rec5[1].Value, "B");
                assert.equal(rec5[1].$id, 1);
                assert.equal(rec5[1].$fq, 5);
                assert.equal(rec5[2].Value, "D");
                assert.equal(rec5[2].$id, 3);
                assert.equal(rec5[2].$fq, 2);

                var rec6 = base.search({ $from: "TestStore2", Value: "y" }).join("Join");
                assert.equal(rec6.length, 2);
                assert.equal(rec6[0].Value, "A");
                assert.equal(rec6[0].$id, 0);
                assert.equal(rec6[0].$fq, 2);
                assert.equal(rec6[1].Value, "B");
                assert.equal(rec6[1].$id, 1);
                assert.equal(rec6[1].$fq, 6);

                var rec7 = base.search({ $from: "TestStore2", Value: "z" }).join("Join");
                assert.equal(rec7.length, 2);
                assert.equal(rec7[0].Value, "A");
                assert.equal(rec7[0].$id, 0);
                assert.equal(rec7[0].$fq, 4);
                assert.equal(rec7[1].Value, "C");
                assert.equal(rec7[1].$id, 2);
                assert.equal(rec7[1].$fq, 1);
            })
            it('should return correct frequencies for records when doing search join', function () {
                prepareJoinStore()
                // first direct joins, without search
                var rec1 = base.search({ $join: { $name: "Join", $query: { $from: "TestStore1", Value: "A" }}});
                assert.equal(rec1.length, 3);
                assert.equal(rec1[0].Value, "x");
                assert.equal(rec1[0].$id, 0);
                assert.equal(rec1[0].$fq, 1);
                assert.equal(rec1[1].Value, "y");
                assert.equal(rec1[1].$id, 1);
                assert.equal(rec1[1].$fq, 2);
                assert.equal(rec1[2].Value, "z");
                assert.equal(rec1[2].$id, 2);
                assert.equal(rec1[2].$fq, 4);

                var rec2 = base.search({ $join: { $name: "Join", $query: { $from: "TestStore1", Value: "B" }}});
                assert.equal(rec2.length, 2);
                assert.equal(rec2[0].Value, "x");
                assert.equal(rec2[0].$id, 0);
                assert.equal(rec2[0].$fq, 5);
                assert.equal(rec2[1].Value, "y");
                assert.equal(rec2[1].$id, 1);
                assert.equal(rec2[1].$fq, 6);

                var rec3 = base.search({ $join: { $name: "Join", $query: { $from: "TestStore1", Value: "C" }}});
                assert.equal(rec3.length, 1);
                assert.equal(rec3[0].Value, "z");
                assert.equal(rec3[0].$id, 2);
                assert.equal(rec3[0].$fq, 1);

                var rec4 = base.search({ $join: { $name: "Join", $query: { $from: "TestStore1", Value: "D" }}});
                assert.equal(rec4.length, 1);
                assert.equal(rec4[0].Value, "x");
                assert.equal(rec4[0].$id, 0);
                assert.equal(rec4[0].$fq, 2);

                var rec5 = base.search({ $join: { $name: "Join", $query: { $from: "TestStore2", Value: "x" }}});
                assert.equal(rec5.length, 3);
                assert.equal(rec5[0].Value, "A");
                assert.equal(rec5[0].$id, 0);
                assert.equal(rec5[0].$fq, 1);
                assert.equal(rec5[1].Value, "B");
                assert.equal(rec5[1].$id, 1);
                assert.equal(rec5[1].$fq, 5);
                assert.equal(rec5[2].Value, "D");
                assert.equal(rec5[2].$id, 3);
                assert.equal(rec5[2].$fq, 2);

                var rec6 = base.search({ $join: { $name: "Join", $query: { $from: "TestStore2", Value: "y" }}});
                assert.equal(rec6.length, 2);
                assert.equal(rec6[0].Value, "A");
                assert.equal(rec6[0].$id, 0);
                assert.equal(rec6[0].$fq, 2);
                assert.equal(rec6[1].Value, "B");
                assert.equal(rec6[1].$id, 1);
                assert.equal(rec6[1].$fq, 6);

                var rec7 = base.search({ $join: { $name: "Join", $query: { $from: "TestStore2", Value: "z" }}});
                assert.equal(rec7.length, 2);
                assert.equal(rec7[0].Value, "A");
                assert.equal(rec7[0].$id, 0);
                assert.equal(rec7[0].$fq, 4);
                assert.equal(rec7[1].Value, "C");
                assert.equal(rec7[1].$id, 2);
                assert.equal(rec7[1].$fq, 1);
            })
        })
    }
    testGixFrequency1("full");
    testGixFrequency1("small");

    function testGixFrequency2(gixType) {
        function prepareJoinStore() {
            base.createStore([{
                name: 'TestStore1',
                fields: [ { name: 'Value', type: 'string', primary: true } ],
                joins: [ { name: 'Join', type: 'index', store: 'TestStore2', inverse: 'Join', storage: gixType } ],
                keys: [ { field: 'Value', type: 'value', storage: gixType } ]
            }, {
                name: 'TestStore2',
                fields: [ { name: 'Value', type: 'string', primary: true } ],
                joins: [ { name: 'Join', type: 'index', store: 'TestStore1', inverse: 'Join', storage: gixType } ],
                keys: [ { field: 'Value', type: 'value', storage: gixType } ]
            }]);
            var store1 = base.store("TestStore1");
            store1.push({ Value: "A" });
            store1.push({ Value: "B" });
            store1.push({ Value: "C" });
            store1.push({ Value: "D" });
            var store2 = base.store("TestStore2");
            store2.push({ Value: "x" });
            store2.push({ Value: "y" });
            store2.push({ Value: "z" });
            store1[0].$addJoin("Join", store2[0]); // A->x
            store1[1].$addJoin("Join", store2[0]); // B->x
            store1[2].$addJoin("Join", store2[0]); // C->x
            store1[3].$addJoin("Join", store2[0]); // D->x
            store1[0].$addJoin("Join", store2[1]); // A->y
            store1[1].$addJoin("Join", store2[1]); // B->y
            store1[2].$addJoin("Join", store2[1]); // C->y
            store1[0].$addJoin("Join", store2[2]); // A->z
            store1[1].$addJoin("Join", store2[2]); // B->z
        }
        describe('Test implicit frequencies for gix type "' + gixType + '"', function () {
            it('should return correct frequencies for records when doing direct join on record', function () {
                prepareJoinStore()
                var rec1 = base.store("TestStore1").allRecords.join("Join");
                assert.equal(rec1.length, 3);
                assert.equal(rec1[0].Value, "x");
                assert.equal(rec1[0].$id, 0);
                assert.equal(rec1[0].$fq, 4);
                assert.equal(rec1[1].Value, "y");
                assert.equal(rec1[1].$id, 1);
                assert.equal(rec1[1].$fq, 3);
                assert.equal(rec1[2].Value, "z");
                assert.equal(rec1[2].$id, 2);
                assert.equal(rec1[2].$fq, 2);
                var rec2 = base.store("TestStore2").allRecords.join("Join");
                assert.equal(rec2.length, 4);
                assert.equal(rec2[0].Value, "A");
                assert.equal(rec2[0].$id, 0);
                assert.equal(rec2[0].$fq, 3);
                assert.equal(rec2[1].Value, "B");
                assert.equal(rec2[1].$id, 1);
                assert.equal(rec2[1].$fq, 3);
                assert.equal(rec2[2].Value, "C");
                assert.equal(rec2[2].$id, 2);
                assert.equal(rec2[2].$fq, 2);
                assert.equal(rec2[3].Value, "D");
                assert.equal(rec2[3].$id, 3);
                assert.equal(rec2[3].$fq, 1);

            })
        })
    }
    testGixFrequency2("full");
    testGixFrequency2("small");
    testGixFrequency2("tiny");
})

describe('Gix Position Tests', function () {
    var base = undefined;

    beforeEach(function () {
        qm.delLock();
        base = new qm.Base({mode: 'createClean'});
    });
    afterEach(function () {
        base.close();
    });

    var text = [
        "Kraft wins first individual ski flying event at Planica",
        "Norway win ski flying team event in Planica, Slovenia fifth",
        "Kraft clinches ski jump title with final-event win",
        "Kraft wins final Planica event and ski-jumping World Cup",
        "Germany, Norway neck-and-neck after the first series in Planica",
        "aa aa aa aa aa bb aa aa aa aa aa cc aa dd",
        "kk " + Array(1022).join("xx ") + "kk mm mm nn",
        "oo pp " + Array(1022).join("rr ") + "ss tt uu"
    ]

    describe('Test creating stores with position index', function () {
        it('create store with position index', function () {
            var store = base.createStore({
                name: 'TestStore',
                fields: [ { name: 'Value', type: 'string' } ],
                keys: [ { field: 'Value', type: 'text_position' } ]
            });
        });
        it('create store with position index on wrong field type', function () {
            assert.throws(function () {
                var store = base.createStore({
                    name: 'TestStore',
                    fields: [ { name: 'Value', type: 'int' } ],
                    keys: [ { field: 'Value', type: 'text_position' } ]
                });
            });
        });
        it('index new records', function () {
            var store = base.createStore({
                name: 'TestStore',
                fields: [ { name: 'Value', type: 'string' } ],
                keys: [ { field: 'Value', type: 'text_position' } ]
            });
            for (var i=0; i < text.length; i++) {
                store.push({ Value: text[i] });
            }
            assert.equal(store.length, text.length);
        });
        it('delete existing records', function () {
            var store = base.createStore({
                name: 'TestStore',
                fields: [ { name: 'Value', type: 'string' } ],
                keys: [ { field: 'Value', type: 'text_position' } ]
            });
            for (var i=0; i < text.length; i++) {
                store.push({ Value: text[i] });
            }
            store.clear(1);
            assert.equal(store.length, text.length - 1);
            store.clear(2);
            assert.equal(store.length, text.length - 3);
            store.clear(text.length - 3);
            assert.equal(store.length, 0);
        });
    });

    describe('Test searching', function () {
        function populate(store) {
            for (var i=0; i < text.length; i++) {
                store.push({ Value: text[i] });
            }
        }

        it('find single word', function () {
            var store = base.createStore({
                name: 'TestStore',
                fields: [ { name: 'Value', type: 'string' } ],
                keys: [ { field: 'Value', type: 'text_position' } ]
            });
            populate(store);

            assert.equal(base.search({ $from: "TestStore", Value: "kraft" }).length, 3);
            assert.equal(base.search({ $from: "TestStore", Value: "aa" }).length, 1);
            assert.equal(base.search({ $from: "TestStore", Value: "aa" })[0].$fq, 11);
            assert.equal(base.search({ $from: "TestStore", Value: "pizza" }).length, 0);
        });
        it('two word phrases', function () {
            var store = base.createStore({
                name: 'TestStore',
                fields: [ { name: 'Value', type: 'string' } ],
                keys: [ { field: 'Value', type: 'text_position' } ]
            });
            populate(store);

            assert.equal(base.search({ $from: "TestStore", Value: "kraft wins" }).length, 2);
            assert.equal(base.search({ $from: "TestStore", Value: "planica event" }).length, 1);
            assert.equal(base.search({ $from: "TestStore", Value: "planica tralala" }).length, 0);
            assert.equal(base.search({ $from: "TestStore", Value: "tralala planica" }).length, 0);
            assert.equal(base.search({ $from: "TestStore", Value: "aa aa" }).length, 1);
            assert.equal(base.search({ $from: "TestStore", Value: "aa aa" })[0].$fq, 8);
            assert.equal(base.search({ $from: "TestStore", Value: "aa bb" }).length, 1);
            assert.equal(base.search({ $from: "TestStore", Value: "aa cc" }).length, 1);
            assert.equal(base.search({ $from: "TestStore", Value: "kk xx" }).length, 1);
            // no items where kk and ll are together
            assert.equal(base.search({ $from: "TestStore", Value: "kk ll" }).length, 0);
            // one valid and one false positive where kk and mm are together - due to modulo 1023
            assert.equal(base.search({ $from: "TestStore", Value: "kk mm" }).length, 1);
            assert.equal(base.search({ $from: "TestStore", Value: "kk mm" })[0].$fq, 2);
            assert.equal(base.search({ $from: "TestStore", Value: "kk nn" }).length, 0);

            assert.equal(base.search({ $from: "TestStore", Value: "xx" }).length, 1);
            assert.equal(base.search({ $from: "TestStore", Value: "xx" })[0].$fq, 1021);

            // false positives (due to modulo positions)
            assert.equal(base.search({ $from: "TestStore", Value: "oo tt" }).length, 1);
            assert.equal(base.search({ $from: "TestStore", Value: "ss pp" }).length, 1);
            assert.equal(base.search({ $from: "TestStore", Value: "tt rr" }).length, 1);

            assert.equal(base.search({ $from: "TestStore", Value: "ss rr" }).length, 0);
            assert.equal(base.search({ $from: "TestStore", Value: { $str: "ss rr", $diff: 2 } }).length, 1);
            assert.equal(base.search({ $from: "TestStore", Value: { $str: "ss rr", $diff: 3 } }).length, 1);
            // overlapping indices
            assert.equal(base.search({ $from: "TestStore", Value: "oo ss" }).length, 0);
            assert.equal(base.search({ $from: "TestStore", Value: "pp tt" }).length, 0);

        });
        it('long word phrases', function () {
            var store = base.createStore({
                name: 'TestStore',
                fields: [ { name: 'Value', type: 'string' } ],
                keys: [ { field: 'Value', type: 'text_position' } ]
            });
            populate(store);

            assert.equal(base.search({ $from: "TestStore", Value: "Kraft wins first individual ski flying event at Planica" }).length, 1);
            assert.equal(base.search({ $from: "TestStore", Value: "planica event" }).length, 1);
            assert.equal(base.search({ $from: "TestStore", Value: "aa aa aa" }).length, 1);
            assert.equal(base.search({ $from: "TestStore", Value: "aa aa aa" })[0].$fq, 6);
        });
        it('phrases with gaps', function () {
            var store = base.createStore({
                name: 'TestStore',
                fields: [ { name: 'Value', type: 'string' } ],
                keys: [ { field: 'Value', type: 'text_position' } ]
            });
            populate(store);

            assert.equal(base.search({ $from: "TestStore", Value: { $str: "kraft planica", $diff: 1 }}).length, 0);
            assert.equal(base.search({ $from: "TestStore", Value: { $str: "kraft planica", $diff: 2 }}).length, 0);
            assert.equal(base.search({ $from: "TestStore", Value: { $str: "kraft planica", $diff: 3 }}).length, 1);
            assert.equal(base.search({ $from: "TestStore", Value: { $str: "cc dd", $diff: 2 }}).length, 1);
        });

    });
});

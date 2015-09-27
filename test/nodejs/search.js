/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

console.log(__filename)
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
              { 'name': 'Value', 'type': 'int' }
            ],
            'joins': [],
            'keys': [
                { field: 'Value', type: 'linear' }
            ]
        });
        store = base.store('BTreeSearchTest');
        for (var i = 0; i < 100; i++) {
            store.push({ Value: i % 10 });
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
              { 'name': 'Value', 'type': 'uint64' }
            ],
            'joins': [],
            'keys': [
                { field: 'Value', type: 'linear' }
            ]
        });
        store = base.store('BTreeSearchTest');
        for (var i = 0; i < 100; i++) {
            store.push({ Value: i % 10 });
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
              { 'name': 'Value', 'type': 'float' }
            ],
            'joins': [],
            'keys': [
                { field: 'Value', type: 'linear' }
            ]
        });
        store = base.store('BTreeSearchTest');
        for (var i = 0; i < 100; i++) {
            store.push({ Value: (i % 10) + 0.123 });
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
              { 'name': 'Value', 'type': 'datetime' }
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
            store.push({ Value: _date.toISOString() });
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

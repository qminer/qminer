/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

// console.log(__filename)
var assert = require('../../src/nodejs/scripts/assert.js'); //adds assert.run function
var qm = require('../../index.js');

describe('Circular Record Buffer Tests', function () {
    var base = undefined;
    var store = undefined;

    beforeEach(function () {
        qm.delLock();
        var backward = require('../../src/nodejs/scripts/backward.js');
        backward.addToProcess(process); // adds process.isArg function

        base = new qm.Base({mode: "createClean"});
		// prepare test set
        base.createStore({
            "name": "CircularRecordBufferTest",
            "fields": [
              { "name": "Value", "type": "float" },
            ],
            "joins": [],
            "keys": []
        });
        store = base.store("CircularRecordBufferTest");
        for (var val = 0; val < 100; val++) {
            store.push({ Value: val });
        }
    });
    afterEach(function () {
        base.close();
    });

    describe('Constructor Tests', function () {
        it('should construct a new buffer using a store and size', function () {
            var circularRecordBuffer = new qm.CircularRecordBuffer({ store: store, size: 10 });
            assert.notEqual(circularRecordBuffer, null);
        })
        it('should throw an exception if store is not given', function () {
            assert.throws(function () {
                var circularRecordBuffer = new qm.CircularRecordBuffer({ size: 10 });
            })
        })
        it('should throw an exception object other that qm.Store is given as store', function () {
            assert.throws(function () {
                var circularRecordBuffer = new qm.CircularRecordBuffer({ store: "Tralala", size: 10 });
            })
        })
        it('should throw an exception if size is not given', function () {
            assert.throws(function () {
                var circularRecordBuffer = new qm.CircularRecordBuffer({ store: store });
            })
        })
        it('should throw an exception object other that number is given as size', function () {
            assert.throws(function () {
                var circularRecordBuffer = new qm.CircularRecordBuffer({ store: store, size: "tralala" });
            })
        })
        it('should throw an exception object invalid number is given as size', function () {
            assert.throws(function () {
                var circularRecordBuffer = new qm.CircularRecordBuffer({ store: store, size: -1 });
            })
        })
    });

    describe('CircularRecordBuffer.push Tests', function () {
        it('should call add 10 times and never delete', function () {
            var countAdd = 0, countDel = 0;
            var circularRecordBuffer = new qm.CircularRecordBuffer({
                store: store, size: 10,
                onAdd: function () { countAdd++; },
                onDelete: function () { countDel++; }
            });
            for (var i = 0; i < 10; i++) { circularRecordBuffer.push(store[i]); }
            assert.equal(countAdd, 10);
            assert.equal(countDel, 0);
        })
        it('should call add 100 times and delete 90 times', function () {
            var countAdd = 0, countDel = 0;
            var circularRecordBuffer = new qm.CircularRecordBuffer({
                store: store, size: 10,
                onAdd: function () { countAdd++; },
                onDelete: function () { countDel++; }
            });
            store.each(function (rec) { circularRecordBuffer.push(rec); });
            assert.equal(countAdd, 100);
            assert.equal(countDel, 90);
        })
        it('should return records in correct sequence', function () {
            var countAdd = 0, countDel = 0;
            var circularRecordBuffer = new qm.CircularRecordBuffer({
                store: store, size: 10,
                onAdd: function (rec) {
                    assert.equal(rec.$id, countAdd);
                    countAdd++;
                },
                onDelete: function (rec) {
                    assert.equal(rec.$id, countDel);
                    countDel++;
                }
            });
            store.each(function (rec) { circularRecordBuffer.push(rec); });
            assert.equal(countAdd, 100);
            assert.equal(countDel, 90);
        })
    });

});

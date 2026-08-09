/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

// console.log(__filename)
var assert = require('../../src/nodejs/scripts/assert.js');     //adds assert.run function
var ht = require('../../index.js').ht;
var fs = require('../../index.js').fs;

///////////////////////////////////////////////////////////////////////////////
// Hash table tests

describe('int float map tests', function () {

    var ifMap;
    beforeEach(function (done) {
        ifMap = new ht.IntFltMap();
        // Adding two key/dat pairs
        ifMap.put(15, 20.2);
        ifMap.put(5, 10.5);
        done();
    });
    afterEach(function (done) {
        done();
    });

    describe('Length Test', function () {
        it('should return the number of records', function () {
            assert.strictEqual(ifMap.length, 2);
        })
    });

    describe('Dat Test', function () {
        it('should the data with the given id', function () {
            assert.strictEqual(ifMap.dat(1), 10.5);
        })
    });

    describe('Key Test', function () {
        it('should return the gey with the given id', function () {
            assert.strictEqual(ifMap.key(1), 5);
        })
    });

    describe('Get Test', function () {
        it('should return the dat of the given key', function () {
            assert.strictEqual(ifMap.get(15), 20.2);
        })
    });

    describe('Has Key Test', function () {
        it('should return true if it has the given key, false otherwise', function () {
            assert.strictEqual(ifMap.hasKey(15), true);
            assert.strictEqual(ifMap.hasKey(100), false);
        })
    });

    describe('Key Id Test', function () {
        it('should return the id of the given key', function () {
            assert.strictEqual(ifMap.keyId(15), 0);
            assert.strictEqual(ifMap.keyId(5), 1);
        })
    });

    describe('Put Test', function () {
        it('should add elements to the hash table', function () {
            assert.strictEqual(ifMap.length, 2);
            ifMap.put(3, 5);
            assert.strictEqual(ifMap.length, 3);
            ifMap.put(3, 5);
            assert.strictEqual(ifMap.length, 3);
        })
    });

     describe('Sort By Dat Test', function () {
        it('should sort the elements ascendently by dat', function () {
            ifMap.sortDat();
            assert.strictEqual(ifMap.dat(1), 20.2);
            ifMap.put(3, 5);
            ifMap.sortDat();
            assert.strictEqual(ifMap.dat(0), 5);
        })
    });
    describe('Sort By Key Test', function () {
        it('should sort the elements ascendently by key', function () {
            ifMap.sortKey();
            assert.strictEqual(ifMap.key(1), 15);
            ifMap.put(1, 10.5);
            ifMap.sortKey();
            assert.strictEqual(ifMap.key(0), 1);
        })
    });
    describe('Save/Load Test', function () {
        it('should save and load the ht', function () {
            fout = fs.openWrite('map.dat'); // open write stream
            // check the contents of the map
            assert.strictEqual(ifMap.length, 2);
            assert.strictEqual(ifMap.keyId(15), 0);
            assert.strictEqual(ifMap.keyId(5), 1);
            assert.strictEqual(ifMap.dat(0), 20.2);
            assert.strictEqual(ifMap.dat(1), 10.5);

            ifMap.save(fout).close(); // save and close write stream

            var ifMap2 = new ht.IntFltMap(); // new empty table
            var fin = fs.openRead('map.dat'); // open read stream
            ifMap2.load(fin); // load

            // check the contents of the new map
            assert.strictEqual(ifMap2.length, 2);
            assert.strictEqual(ifMap2.keyId(15), 0);
            assert.strictEqual(ifMap2.keyId(5), 1);
            assert.strictEqual(ifMap2.dat(0), 20.2);
            assert.strictEqual(ifMap2.dat(1), 10.5);
        })
    });
})

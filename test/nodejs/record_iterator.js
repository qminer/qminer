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
qm.delLock();

// the database/store, from which we get the record set
function TStore() {
    this.base = new qm.Base({ mode: 'createClean' });
    this.base.createStore([{
        "name": "People",
        "fields": [
            { "name": "Name", "type": "string", "primary": true },
            { "name": "Gender", "type": "string", "shortstring": true }
        ],
        "keys": [
            { "field": "Name", "type": "text" },
            { "field": "Gender", "type": "value" }
        ]
    }]);
    // adding two persons
    this.base.store("People").push({ "Name": "Carolina Fortuna", "Gender": "Female" });
    this.base.store("People").push({ "Name": "Blaz Fortuna", "Gender": "Male" });

    this.close = function () {
        this.base.close();
    }
};

///////////////////////////////////////////////////////////////////////////////
// Records

describe('Records Tests', function () {

    var table, rec;
    beforeEach(function () {
        table = new TStore();
        rec = table.base.store("People")[0];
    });
    afterEach(function () {
        table.close();
    });

    describe('Clone Tests', function () {
        it('should clone the record', function () {
            var rec2 = rec.$clone();
            assert.equal(rec2.Name, rec.Name);
            assert.equal(rec2.Gender, rec.Gender);
        })
    });

    describe('ToJSON Tests', function () {
        it('should return the record as a JSON', function () {
            var json = rec.toJSON();
            assert.equal(json.$id, 0);
            assert.equal(json.Name, "Carolina Fortuna");
            assert.equal(json.Gender, "Female");
        })
    });

    describe('Id Test', function () {
        it('should return the id of the record', function () {
            assert.equal(rec.$id, 0);
        })
    });

    describe('Name Test', function () {
        it('should return the name of the record', function () {
            assert.equal(rec.$name, "Carolina Fortuna");
        })
    });

    describe('Fq Test', function () {
        it('should return the frequency of the record', function () {
            assert.equal(rec.$fq, 1);
        })
    });

    describe('Store Test', function () {
        it('should return the store the record belongs to', function () {
            assert.equal(rec.$store.name, "People");
        })
    });
});

///////////////////////////////////////////////////////////////////////////////
// Store Iterator

describe('StoreIterator Tests', function () {
    var table, iter;
    beforeEach(function () {
        table = new TStore();
        iter = table.base.store("People").forwardIter;
    });
    afterEach(function () {
        table.close();
    });

    describe('Next Test', function () {
        it('should give the first record of the store', function () {
            assert.equal(iter.next(), true);
            assert.equal(iter.record.Name, "Carolina Fortuna");
            assert.equal(iter.record.Gender, "Female");
            assert.equal(iter.store.name, "People");
        })
        it('should go through all the records of the store', function () {
            assert.equal(iter.next(), true);
            assert.equal(iter.record.Name, "Carolina Fortuna");
            assert.equal(iter.record.Gender, "Female");
            assert.equal(iter.store.name, "People");

            assert.equal(iter.next(), true);
            assert.equal(iter.record.Name, "Blaz Fortuna");
            assert.equal(iter.record.Gender, "Male");
            assert.equal(iter.store.name, "People");

            assert.equal(iter.next(), false);
        })
    })
})

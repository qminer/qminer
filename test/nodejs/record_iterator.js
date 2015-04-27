/**
 * Copyright (c) 2015, Quintelligence d.o.o.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 */
 
console.log(__filename)
var assert = require('../../src/nodejs/scripts/assert.js');     //adds assert.run function
var qm = require('qminer');
qm.delLock();

qm.config('qm.conf', true, 8080, 1024);

// the database/store, from which we get the record set
function TStore() {
    this.base = qm.create('qm.conf', "", true);
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
    this.base.store("People").add({ "Name": "Carolina Fortuna", "Gender": "Female", $fq: 5 });
    this.base.store("People").add({ "Name": "Blaz Fortuna", "Gender": "Male", $fq: 3 });

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
        it.skip('should return the frequency of the record', function () {
            assert.equal(rec.$fq, 5);
        })
    });

    describe('Store Test', function () {
        it.skip('should return the store the record belongs to', function () {
            assert.equal(rec.$store, "People");
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
            assert.equal(iter.rec.Name, "Carolina Fortuna");
            assert.equal(iter.rec.Gender, "Female");
            assert.equal(iter.store.name, "People");
        })
        it('should go through all the records of the store', function () {
            assert.equal(iter.next(), true);
            assert.equal(iter.rec.Name, "Carolina Fortuna");
            assert.equal(iter.rec.Gender, "Female");
            assert.equal(iter.store.name, "People");

            assert.equal(iter.next(), true);
            assert.equal(iter.rec.Name, "Blaz Fortuna");
            assert.equal(iter.rec.Gender, "Male");
            assert.equal(iter.store.name, "People");

            assert.equal(iter.next(), false);
        })
    })
})
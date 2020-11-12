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

describe('Record Vector Tests', function () {
    var base, store;
    beforeEach(function (done) {
        base = new qm.Base({
            mode: 'createClean',
            schema: [{
                name: "Philosophers",
                fields: [
                    { name: "Name", type: "string" },
                    { name: "Era", type: "string" }
                ]
            }]
        });
        store = base.store("Philosophers");
        done();
    });
    afterEach(function (done) {
        base.close();
        done();
    });

    describe('Constructor tests', function () {
        it('should create empty record vector', function () {
            var recordVector = new qm.RecordVector(base);
            assert.strictEqual(recordVector.length, 0);
        })
        it('should throw exception', function () {
            assert.throws(function () { new qm.RecordVector(); });
            assert.throws(function () { new qm.RecordVector({}); });
            assert.throws(function () { new qm.RecordVector(base, {}); });
        })
    });
    describe('Managing records', function () {
        it('should create record vector with two records', function () {
            var recordVector = new qm.RecordVector(base);
            assert.strictEqual(recordVector[0], null);
            assert.strictEqual(recordVector.length, 0);
            recordVector.push(store.newRecord({ Name: "Plato", Era: "Ancient philosophy" }));
            assert.strictEqual(recordVector.length, 1);
            assert.strictEqual(recordVector[0].Name, "Plato");
            assert.strictEqual(recordVector[0].Era, "Ancient philosophy");
            assert.strictEqual(recordVector[1], null);
            var record = store.newRecord({ Name: "Immanuel Kant", Era: "18th-century philosophy" });
            recordVector.push(record);
            assert.strictEqual(recordVector.length, 2);
            assert.strictEqual(recordVector[1].Name, "Immanuel Kant");
            assert.strictEqual(recordVector[1].Era, "18th-century philosophy");
            assert.strictEqual(recordVector[2], null);
        })
    });
    describe('Serialization tests', function () {
        it('should save vector', function () {
            var recordVector = new qm.RecordVector(base);
            recordVector.push(store.newRecord({ Name: "Plato", Era: "Ancient philosophy" }));
            recordVector.push(store.newRecord({ Name: "Immanuel Kant", Era: "18th-century philosophy" }));
            var fout = qm.fs.openWrite('record_vector.bin');
            recordVector.save(fout).close();
        })
        it('should save and load vector', function () {
            var outRecordVector = new qm.RecordVector(base);
            outRecordVector.push(store.newRecord({ Name: "Plato", Era: "Ancient philosophy" }));
            outRecordVector.push(store.newRecord({ Name: "Immanuel Kant", Era: "18th-century philosophy" }));
            var fout = qm.fs.openWrite('record_vector.bin');
            outRecordVector.save(fout).close();

            var fin = qm.fs.openRead('record_vector.bin');
            var inRecordVector = new qm.RecordVector(base, fin);
            assert.strictEqual(inRecordVector.length, 2);
            assert.strictEqual(inRecordVector[0].Name, "Plato");
            assert.strictEqual(inRecordVector[0].Era, "Ancient philosophy");
            assert.strictEqual(inRecordVector[1].Name, "Immanuel Kant");
            assert.strictEqual(inRecordVector[1].Era, "18th-century philosophy");
            assert.strictEqual(inRecordVector[2], null);
        })
    });

})

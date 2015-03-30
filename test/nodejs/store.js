console.log(__filename)
var assert = require('../../src/nodejs/scripts/assert.js');     //adds assert.run function
var qm = require('qminer');
qm.delLock();
//qm.rmDir('db') // run from qminer/test/nodejs 

qm.config('qm.conf', true, 8080, 1024);

function EmptyStore() {
    // creates the base and store
    this.base = qm.create('qm.conf', "", true);
    this.base.createStore({
        "name": "People",
        "fields": [
            { "name": "Name", "type": "string", "primary": true },
            { "name": "Gender", "type": "string", "shortstring": true }
        ]
    });

    // adds a person in the store
    this.add = function () {
        this.base.store("People").add({ "Name": "Carolina Fortuna", "Gender": "Female" });
    }

    // closes the store
    this.close = function () {
        this.base.close();
    }
}
describe('Empty Store Tests', function () {

    var table = undefined;
    beforeEach(function () {
        table = new EmptyStore();
    });
    afterEach(function () {
        table.close();
    });

    describe('Created Test', function () {
        it('should return true if store is not null', function () {
            assert(null != table.base.store("People"));
        })
    });
    describe('Name Test', function () {
        it('should return "People" as store name', function () {
            assert.equal(table.base.store("People").name, "People");
        })
    });

    describe('Length Test', function () {
        it('should return 0', function () {
            assert.equal(table.base.store("People").length, 0);
        })
    });

    describe('Recs Test', function () {
        it('should return a json with no elements', function () {
            var recs = table.base.store("People").recs;
            assert(recs.empty);
            assert.equal(recs.length, 0);
        })
    });

    describe('Empty Test', function () {
        it('should return true for an empty store', function () {
            assert(table.base.store("People").empty);
        })
    })

    describe('Fields Test', function () {
        it('should return the number of fields', function () {
            var arr = table.base.store("People").fields;
            assert.equal(arr.length, 2);
        })
        it('should return an array of all Fields', function () {
            var arr = table.base.store("People").fields;
            assert.equal(arr[0].name, "Name");
            assert.equal(arr[1].name, "Gender");
        })
    });

    describe('Joins Test', function () {
        it('should return an empty array', function () {
            var arr = table.base.store("People").joins;
            assert.deepEqual(arr, []);
        })
    });

    describe('Keys Test', function () {
        it('should return an empty array', function () {
            var arr = table.base.store("People").joins;
            assert.deepEqual(arr, []);
        })
    });

    describe('First Test', function () {
        it('should return a null object', function () {
            var rec = table.base.store("People").first;
            assert(rec == null);
        })
    });

    describe('Last Test', function () {
        it('should return a null object', function () {
            var rec = table.base.store("People").last;
            assert(rec == null);
        })
    });

    describe('ForwardIter Test', function () {
        it('should return an empty json', function () {
            var iter = table.base.store("People").forwardIter;
            assert.equal(iter.store.length, 0);
        })
    });

    describe('BackwardIter Test', function () {
        it('should return an empty json', function () {
            var iter = table.base.store("People").backwardIter;
            assert.equal(iter.store.length, 0);
        })
    })
})
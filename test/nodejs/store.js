console.log(__filename)
var assert = require('../../src/nodejs/scripts/assert.js');     //adds assert.run function
var qm = require('qminer');
qm.delLock();
//qm.rmDir('db') // run from qminer/test/nodejs 

qm.config('qm.conf', true, 8080, 1024);

///////////////////////////////////////////////////////////////////////////////////////////////////
// Empty Store

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
            var iter = table.base.store("People").forwardIter.next();
            assert(!iter);
        })
    });

    describe('BackwardIter Test', function () {
        it('should return an empty json', function () {
            var iter = table.base.store("People").backwardIter;
            assert.equal(iter.store.length, 0);
        })
    });

    describe('Add Test', function () {
        it('should add a new person in store', function () {
            assert.equal(table.base.store("People").add({ "Name": "Carolina Fortuna", "Gender": "Female" }), 0);
            assert.equal(table.base.store("People").length, 1);
        })
    });
})

///////////////////////////////////////////////////////////////////////////////////////////////////
// Small Store

function Store() {
    this.base = qm.create('qm.conf', "", true);
    this.base.createStore({
        "name": "People",
        "fields": [
            { "name": "Name", "type": "string", "primary": true },
            { "name": "Gender", "type": "string", "shortstring": true }
        ]
    });
    this.base.store("People").add({ "Name": "Carolina Fortuna", "Gender": "Female" });
    this.base.store("People").add({ "Name": "Blaz Fortuna", "Gender": "Male" });

    this.close = function () {
        this.base.close();
    }
};

describe('Store Tests', function () {

    var table = undefined;
    beforeEach(function () {
        table = new Store();
    });
    afterEach(function () {
        table.close();
    });

    describe('Length Test', function () {
        it('should return the number of persons in store', function () {
            assert.equal(table.base.store("People").length, 2);
        })
    });

    describe('Name Test', function () {
        it('should return the name of both persons', function () {
            assert.equal(table.base.store("People")[0].Name, "Carolina Fortuna");
            assert.equal(table.base.store("People")[1].Name, "Blaz Fortuna");
        })
    });

    describe('Gender Test', function () {
        it('should return the genders of both persons', function () {
            assert.equal(table.base.store("People")[0].Gender, "Female");
            assert.equal(table.base.store("People")[1].Gender, "Male");
        })
    });

    describe('ForwardIter Test', function () {
        it('should go through the persons in store', function () {
            var PeopleIter = table.base.store("People").forwardIter;
            assert.equal(PeopleIter.next(), true);
            assert.equal(PeopleIter.rec.$id, 0);
            assert.equal(PeopleIter.next(), true);
            assert.equal(PeopleIter.rec.$id, 1);
            assert.equal(PeopleIter.next(), false);
        })
    })

})
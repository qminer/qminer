var qm = require('qminer');
var assert = require("../../src/nodejs/scripts/assert.js");

// newRecord Unit Tests
describe("newRecord test", function () {
    var base = undefined;
    beforeEach(function () {
        base = new qm.Base({
            mode: 'createClean',
            schema: [{
                name: 'People',
                fields: [{ name: 'name', type: 'string', primary: true }],
                joins: [{ name: 'directed', 'type': 'index', 'store': 'Movies', 'inverse': 'director' }]
            },
            {
                name: 'Movies',
                fields: [{ name: 'title', type: 'string', primary: true }],
                joins: [{ name: 'director', 'type': 'field', 'store': 'People', 'inverse': 'directed' }]
            }]
        });
    });
    afterEach(function () {
        base.close();
    });
    it("should create a record using the index join", function () {       
        var newPerson1 = base.store("People").newRecord({ name: "Peter Jackson" })
        assert.equal(newPerson1.directed.length, 0);

        base.store("Movies").push({title: "Hobbit"});
        var newPerson2 = base.store("People").newRecord({ name: "Peter Jackson", directed: [{ $name: "Hobbit" }] });
        assert.equal(newPerson2.directed.length, 1);

        base.store("Movies").push({title: "The Lord of the Rings"});
        var newPerson3 = base.store("People").newRecord({ name: "Peter Jackson", directed: [{ $name: "Hobbit" }, { $name: "The Lord of the Rings" }] });
        assert.equal(newPerson3.directed.length, 2);
    });

    it.skip("should create a record using the field join", function () {
        var newMovie = base.store("Movies").newRecord({ "director": { name: "Peter Jackson" } });
        console.log(newMovie.director);

        assert.equal(newMovie.director.name, "Peter Jackson");
    });
});

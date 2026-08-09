var qm = require('../../index.js');
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
        assert.strictEqual(newPerson1.directed.length, 0);

        base.store("Movies").push({title: "Hobbit"});
        var newPerson2 = base.store("People").newRecord({ name: "Peter Jackson", directed: [{ $name: "Hobbit" }] });
        assert.strictEqual(newPerson2.directed.length, 1);

        base.store("Movies").push({title: "The Lord of the Rings"});
        var newPerson3 = base.store("People").newRecord({ name: "Peter Jackson", directed: [{ $name: "Hobbit" }, { $name: "The Lord of the Rings" }] });
        assert.strictEqual(newPerson3.directed.length, 2);
    });

    it.skip("should create a record using the field join", function () {
        var newMovie = base.store("Movies").newRecord({ "director": { name: "Peter Jackson" } });
        console.log(newMovie.director);

        assert.strictEqual(newMovie.director.name, "Peter Jackson");
    });
    it("should create a record when store has a field of type 'json'", function () {
        var basePlanets = new qm.Base({
            mode: "createClean",
            schema: [{
                name: "Planets",
                fields: [
                    { name: "Name", type: "string" },
                    { name: "Diameter", type: "int" },
                    { name: "NearestStars", type: "string_v" },
                    { name: "Json", type: "json" }
                ]
            }]
          });
        planet = basePlanets.store("Planets").newRecord({ Name: "Tatooine", Diameter: 10465, NearestStars: ["Tatoo 1", "Tatoo 2"], Json: {fruit: "banana"} });
        assert.deepEqual(planet.Json, {fruit: "banana"});
    });
});

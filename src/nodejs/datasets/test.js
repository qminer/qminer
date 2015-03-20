// 
// Sample unit test using standard assert JS library 
// 

var assert = require("assert")
var datasets = require('../../../index.js').datasets

describe('qm.datasets', function () {

    describe('#describe()', function () {

        it('it should return text description of the module', function () {
            assert.ok(datasets.description())
        });

    });

    describe('#loadIris()', function () {

        it('it should return store with iris dataset', function () {
            assert.ok(datasets.loadIris())
        });

    })

});

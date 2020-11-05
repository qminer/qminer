/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

// console.log(__filename)
var qm = require('../../index.js');
var assert = require('assert')
var datasets = qm.datasets;

describe('qm.datasets', function () {

    describe('#describe()', function () {

        it('it should return text description of the module', function () {
            assert.ok(datasets.description());
        });

    });

    describe('#loadIrisSync() - testing synchronous version of Iris dataset loader', function () {
        var base;
        var Iris;

        // Create base and load data before tests
        before(function () {
            base = new qm.Base({ mode: 'createClean' });
            Iris = datasets.loadIrisSync(base);
        });

        // Close base after tests
        after(function () {
            base.close();
        });

        it('it should load store with iris dataset', function () {
            assert.ok(Iris.name);
        });

        it('it should return store with name Iris', function () {
            assert.strictEqual(Iris.name, "Iris");
        });

        it('it should return store with 150 records', function () {
            assert.strictEqual(Iris.length, 150);
        });

    })

    describe('#loadIris() - testing asynchronous version of Iris dataset loader', function () {
        var base;
        var Iris;

        // Reload base, and wait for store to be loaded.
        before(function (done) {
            base = new qm.Base({ mode: 'createClean' });

            datasets.loadIris(base, function (err, store) {
                if (err) throw err;
                Iris = store;
                done();
            });

        });

        // Close base after tests
        after(function () {
            base.close();
        })

        it('it should load store with iris dataset', function () {
            assert.ok(Iris.name);
        });

        it('it should return store with name Iris', function () {
            assert.strictEqual(Iris.name, "Iris");
        });

        it('it should return store with 150 records', function () {
            assert.strictEqual(Iris.length, 150);
        });

    });

});

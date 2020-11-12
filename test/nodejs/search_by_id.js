/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

// console.log(__filename)
var assert = require('../../src/nodejs/scripts/assert.js'); //adds assert.run function
var qm = require('../../index.js');

describe('Simple search with id as filter parameter', function () {
    var base = undefined;
    var store = undefined;

    beforeEach(function () {
        qm.delLock();
        base = new qm.Base({mode: 'createClean'});
		// prepare test set
        base.createStore({
            'name': 'TestStore',
            'fields': [ { 'name': 'Value', 'type': 'string' } ],
            'joins': [],
            'keys': [
                { field: 'Value', type: 'value' }
            ]
        });
        store = base.store('TestStore');
        for (var i = 0; i < 100; i++) {
            store.push({ Value: "" + (i % 10) });
        }
    });
    afterEach(function () {
        base.close();
    });

    describe('Tests of base.search after initial push', function () {
        it('returns all elements >= 5', function () {

			// TODO this search is not supported

			//var result = base.search({ $from: 'TestStore', $id: { $gt: 5}});
            //assert.strictEqual(result.length, 94);
        })
    });
});

/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

// console.log(__filename)
var qm = require('../../index.js');

describe('Empty Store Tests', function () {
    var base1 = undefined;
    var base2 = undefined;

    beforeEach(function () {
        base1 = new qm.Base({
            mode: 'createClean',
            schema: [
              {
                  name: 'People',
                  fields: [{ name: 'name', type: 'string', primary: true }],
              }
            ],
            dbPath: './db1'
        });

        base2 = new qm.Base({
            mode: 'createClean',
            schema: [
              {
                  name: 'Movies',
                  fields: [{ name: 'title', type: 'string', primary: true }],
              }
            ],
            dbPath: './db2'
        });
    });
    afterEach(function () {
        base1.close();
        base2.close();
    });

    describe('Two bases in write mode', function () {
        it('insert one record to each base', function () {
            var store1 = base1.store("People");
            store1.push({ name: "Jan" });
            var rec1 = store1.first;
            var store2 = base2.store("Movies");
            store2.push({ title: "Movie" });
            var rec2 = store2.first;

            var assert = require('assert');
            assert(rec1.name == "Jan");
            assert(rec2.title == "Movie");
        });
    });
});

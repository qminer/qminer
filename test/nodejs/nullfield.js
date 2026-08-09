/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

var assert = require('assert');
var qm = require('../../index.js');

describe('nullfield test, old', function () {
	it('should survive', function () {


var base1 = new qm.Base({
    mode: 'createClean',
    schema: [
      {
          name: 'People',
          fields: [{ name: 'name', type: 'string', null: true }],
      }
    ],
    dbPath: './db1'
});

var store1 = base1.store("People");
store1.push({ name: "Jan" });
store1.push({ name: null });
assert(store1.length == 2);
base1.close();
})});
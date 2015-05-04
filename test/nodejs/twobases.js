/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
 
var qm = require('qminer');
var base1 = new qm.Base({
    mode: 'createClean',
    schema: [
      {
          name: 'People',
          fields: [{ name: 'name', type: 'string', primary: true }],          
      }
    ],
    dbPath: './db1'
});

var base2 = new qm.Base({
    mode: 'createClean',
    schema: [
      {
          name: 'Movies',
          fields: [{ name: 'title', type: 'string', primary: true }],          
      }
    ],
    dbPath: './db2'
});

var store1 = base1.store("People");
store1.add({ name: "Jan" });
var rec1 = store1.first;
var store2 = base2.store("Movies");
store2.add({ title: "Movie" });
var rec2 = store2.first;

var assert = require('assert');
assert(rec1.name == "Jan");
assert(rec2.title == "Movie");

base1.close();
base2.close();
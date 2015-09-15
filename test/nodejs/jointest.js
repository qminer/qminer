/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
describe('Join Tests', function () {
    it('should pass', function () {

        var qm = require('qminer');
        var assert = require('assert');
        var base = new qm.Base({
            mode: 'createClean',
            schema: [
              {
                  name: 'People',
                  fields: [{ name: 'name', type: 'string', primary: true }],
                  joins: [{ name: 'friends', 'type': 'index', 'store': 'People' }]
              }
            ]
        });

        var id1 = base.store('People').push({ name: "John" });
        var id2 = base.store('People').push({ name: "Mary" });
        var id3 = base.store('People').push({ name: "Jim" });

        base.store('People')[id1].$addJoin('friends', id2);
        base.store('People')[id1].$addJoin('friends', id3);

        base.store('People')[id2].$addJoin('friends', base.store('People')[id1]);
        base.store('People')[id2].$addJoin('friends', base.store('People')[id2]);

        assert(base.store('People')[id1].friends.length == 2);
        assert(base.store('People')[id2].friends.length == 2);

        //base.store('People')[id2].friends.each(function (rec) { console.log(rec.name); });

        base.close();
    })
});
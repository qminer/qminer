/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

var qm = require('qminer');
var assert = require('assert');

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// index join
describe('Index-Join Test', function () {
    it('should pass', function () {

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

        base.store('People')[id1].$addJoin('friends', id2, 7);
        base.store('People')[id1].$addJoin('friends', id3, 8);

        base.store('People')[id2].$addJoin('friends', base.store('People')[id1], 9);
        base.store('People')[id2].$addJoin('friends', base.store('People')[id2], 3); // narcissism or healthy self-image?

        assert(base.store('People')[id1].friends.length == 2);
        assert(base.store('People')[id1].friends[0].name == "Mary");
        assert(base.store('People')[id1].friends[0].$fq == 7);
        assert(base.store('People')[id1].friends[1].name == "Jim");
        assert(base.store('People')[id1].friends[1].$fq == 8);
        
        assert(base.store('People')[id2].friends.length == 2);
        assert(base.store('People')[id2].friends[0].name == "John");
        assert(base.store('People')[id2].friends[0].$fq == 9);
        assert(base.store('People')[id2].friends[1].name == "Mary");
        assert(base.store('People')[id2].friends[1].$fq == 3);

        //base.store('People')[id2].friends.each(function (rec) { console.log(rec.name); });

        base.close();    
    })
});

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// utility functions

function CreateBase(type, type2) {
    var base = new qm.Base({
        mode: 'createClean',
        schema: [
            {
                name: 'People',
                fields: [{ name: 'name', type: 'string', primary: true }],
                joins: [{ name: 'parent', type: 'field', store: 'People', storage: type + "-" + type2 }]
            }
        ]
    });    
    return base;
}

function FillAndCheck(base, rec_id_type, freq_type) {
    var id1 = base.store('People').push({ name: "John" });
    var id2 = base.store('People').push({ name: "Mary" });
    var id3 = base.store('People').push({ name: "Jim" });

    var rec1 = base.store('People')[id1];
    var rec2 = base.store('People')[id2];
    
    rec1.$addJoin('parent', id2, 7);
    rec2.$addJoin('parent', id3, 8);
    
    // reload data, just in case
    rec1 = base.store('People')[id1];
    rec2 = base.store('People')[id2];
    
    assert(rec1.parent.name == "Mary");
    console.log(rec1.parent.$fq, freq_type);
    assert(rec1.parent.$fq == (freq_type == "" ? 1 : 7));
    
    assert(rec2.parent.name == "Jim");
    assert(rec2.parent.$fq == (freq_type == "" ? 1 : 8));

    base.close();    
}

// field joins
describe('Different join-field-type tests', function () {
    this.timeout(15000);
    it('should pass', function () {
        var rec_id_types = ["uint64", "uint", "uint16", "byte"];
        var freq_types = ["uint16", "byte", "int", "int16", ""];

        for (var i = 0 ; i < rec_id_types.length; i++) {
            for (var j = 0 ; j < freq_types.length; j++) {
                var rec_id_type = rec_id_types[i];
                var freq_type = freq_types[j]; 
                
                console.log(rec_id_type, "-", freq_type);
                var base = CreateBase(rec_id_type, freq_type);
                FillAndCheck(base, rec_id_type, freq_type);
            }    
        }
    })
});
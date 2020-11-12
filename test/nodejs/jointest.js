/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

var qm = require('../../index.js');
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

        // check that filterByField handles index-joins

        var rs_all = base.store('People')
            .allRecords
            .filterByField("friends", id2, id2); // return all that have Mary as friend => John, Mary

        //console.log(rs_all.length);
        assert(rs_all.length == 2);
        assert(rs_all[0].$id == id1);
        assert(rs_all[0].name == "John");
        assert(rs_all[1].$id == id2);
        assert(rs_all[1].name == "Mary");

        base.close();
    })
});

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// utility functions

function CreateBase(type, type2, sloc) {
    console.log(type + "/" + type2 + "/" + sloc); // this output is here to verify that correct combinations are executed
    var base = new qm.Base({
        mode: 'createClean',
        schema: [
            {
                name: 'People',
                fields: [{ name: 'name', type: 'string', primary: true }],
                joins: [{ name: 'parent', type: 'field', store: 'People', storage: type + "-" + type2, storage_location: sloc || "memory" }]
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
    assert(rec1.parent.$fq == (freq_type == "" ? 1 : 7));

    assert(rec2.parent.name == "Jim");
    assert(rec2.parent.$fq == (freq_type == "" ? 1 : 8));

    // perform filterByField

    var rs_all = base.store('People')
        .allRecords
        .filterByField("parent", id2, id2);
    assert(rs_all.length == 1);
    assert(rs_all[0].$id == id1);
    assert(rs_all[0].name == "John");
}
/*
// field joins
describe('Different join-field-type tests', function () {
    it('should pass', function () {
        var rec_id_types = ["uint64", "uint", "uint16", "byte"];
        var freq_types = ["uint16", "byte", "int", "int16", ""];

        for (var k = 0; k < 2; k++) {
            var sloc = (k == 0 ? "memory" : "cache");
            for (var i = 0 ; i < rec_id_types.length; i++) {
                for (var j = 0 ; j < freq_types.length; j++) {
                    var rec_id_type = rec_id_types[i];
                    var freq_type = freq_types[j];

                    //console.log(rec_id_type, "-", freq_type, " ", sloc);
                    var base = CreateBase(rec_id_type, freq_type, sloc);
                    FillAndCheck(base, rec_id_type, freq_type);
                    base.close();
                }
            }
        }
    })
});
*/

var rec_id_types = ["uint64", "uint", "uint16", "byte"];
var freq_types = ["uint16", "byte", "int", "int16", ""];
function SingleStep(_rec_id_type, _freq_type, _sloc) {
    var base = CreateBase(_rec_id_type, _freq_type, _sloc);
    FillAndCheck(base, _rec_id_type, _freq_type);
    base.close();
}

for (var k = 0; k < 2; k++) {
    var sloc = (k == 0 ? "memory" : "cache");
    for (var i = 0 ; i < rec_id_types.length; i++) {
        for (var j = 0 ; j < freq_types.length; j++) {
            var rec_id_type = rec_id_types[i];
            var freq_type = freq_types[j];
            describe('Different join-field-type tests ' + rec_id_type + "/" + freq_type + "/" + sloc, function () {
                it('should pass', function (rec_id_type, freq_type, sloc) {
                    return function() {
                        SingleStep(rec_id_type, freq_type, sloc);
                    };
                }(rec_id_type, freq_type, sloc))
            });
        }
    }
}
/*
{
    console.log("##");
    var rec_id_type = rec_id_types[2];
    var freq_type = freq_types[4];
    var sloc = "memory"; // : "cache");
    var base = CreateBase(rec_id_type, freq_type, sloc);
    FillAndCheck(base, rec_id_type, freq_type);
    console.log("##");

    for (var i = 0; i < 5000; i++) {
        base.store('People').push({ name: "Jim" + i })
    }
    console.log("##");

    base.close();
}
*/
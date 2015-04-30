var qm = require('qminer');
var assert = require('assert');
var base = new qm.Base({
    mode: 'createClean',
    schema: [
      { name: 'People', 
        fields: [{ name: 'name', type: 'string', primary: true }], 
        joins: [{ name: 'friends', 'type': 'index', 'store': 'People'} ]
      }
    ]
});

var id1 = base.store('People').add({name : "John"});
var id2 = base.store('People').add({name : "Mary"});
var id3 = base.store('People').add({name : "Jim"});

base.store('People')[id1].addJoin('friends', id2);
base.store('People')[id1].addJoin('friends', id3);

base.store('People')[id2].addJoin('friends', base.store('People')[id1]);
base.store('People')[id2].addJoin('friends', base.store('People')[id2]);

assert(base.store('People')[id1].friends.length == 2);
assert(base.store('People')[id2].friends.length == 2);

base.store('People')[id2].friends.each(function (rec) {console.log(rec.name)});

base.close();
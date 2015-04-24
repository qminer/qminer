var qm = require('qminer');
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
store1.add({ name: "Jan" });
store1.add({ name: null });
assert(store1.length == 2);
base1.close();

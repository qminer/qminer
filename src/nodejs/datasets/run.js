var qm = require('../../../index.js');
var datasets = qm.datasets;

qm.delLock();
qm.config('qm.conf', true, 8080, 1024);

var base = qm.create('qm.conf', '', true); // 2nd arg: schema, 3rd arg: clear db folder = true

// Test desctiprion function
console.log(datasets.description());

//// Load Iris dataset in async way
//datasets.loadIris(base, function (err, store) {
//    if (err) throw err;
//    console.log("\nSucessfully loaded database: " + store.name)
//    console.log(JSON.stringify(store.first, null, 2));
//})

// Load Iris in sync way.
var Iris = datasets.loadIrisSync(base);
console.log("\nSucessfully loaded database: " + Iris.name)
console.log(JSON.stringify(Iris.first, null, 2));


console.log("\nTESTING TESTING\n")
// If loaded with async way, this output should be displayed before Iris is loaded.
// If loaded with sync way, this output should be displayed after Iris is loaded.

//base.close()

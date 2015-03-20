var qm = require('../../../index.js');
var datasets = qm.datasets;

qm.delLock();
qm.config('qm.conf', true, 8080, 1024);

var base = qm.create('qm.conf', '', true); // 2nd arg: schema, 3rd arg: clear db folder = true

// Test desctiprion function
console.log(datasets.description());


//datasets.loadIris(base, function (err, store) {
//    if (err) throw err;
//    console.log("\nSucessfully loaded database: " + store.name)
//    console.log(JSON.stringify(store.first, null, 2));
//})


// Test loadIris function
var Iris = datasets.loadIrisSync(base); //TODO
console.log("\nSucessfully loaded database: " + Iris.name)
console.log(JSON.stringify(Iris.first, null, 2));


console.log("\nTESTING TESTING\n")

//base.close()

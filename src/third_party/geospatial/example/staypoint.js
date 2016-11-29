var qm = require('qminer');

var base = new qm.Base({
    mode: "createClean",
    schema: [
	    {
	        "name": "GPS",
	        "fields": [
              { "name": "Time", "type": "datetime" },
              { "name": "Location", type: "float_pair" },
              { "name": "Accuracy", type: "byte", "null": true }
	        ],
	        "joins": [],
	        "keys": []
	    }
    ]
});

// used only for schema
// will not be used to hold records (push will not be called)
var store = base.store("GPS");

var aggr = store.addStreamAggr({
    type: "stayPointDetector",
    timeField: "Time",
    locationField: "Location",
    accuracyField: "Accuracy",
    params: { k: 5 }
});

var ts = Date.now();
for (var i = 0; i < 100; i++) {
    // create qminer wrapped record from JSON
    var rec = store.newRecord({
        Time: ts + i,
        Location: [Math.random(), Math.random()],
        Accuracy: 1
    });
    // calls onAdd on all stream aggregates registered on store
    store.triggerOnAddCallbacks(rec);
    var result = aggr.saveJson();
    console.log(result);
    console.log(new Date(result.lastTimestamp));
}


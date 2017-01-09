var qm = require('qminer');

var base = new qm.Base({
    mode: "createClean",
    schema: [
	    {
	        "name": "GPS",
	        "fields": [
              { "name": "Time", "type": "int64" },
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
    params: { dT: 51, tT: 301 }
});

//test1
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

/*
//test2
var points = [
    {//0
        "latitude": 46.0423046,
        "longitude": 14.4875852,
        "time": 0,
        "accuracy": 26
    },//goes to plcos
    {//1
        "latitude": 46.0423046,
        "longitude": 14.4875852,
        "time": 1000,
        "accuracy": 26
    },///goes to plocs then to CL
    {//2
        "latitude": 46.0423046,
        "longitude": 14.4875852,
        "time": 1342444474215,//100000 s later 
        "accuracy": 26
    },//goes to CL
    {//3
        "latitude": 47.0423046,//lat +1 
        "longitude": 14.4875852,
        "time": 13424544474215,//100000 s later 
        "accuracy": 26
    },//goes to PLocs
    {//4
        "latitude": 48.0423046,//lat +1 
        "longitude": 14.4875852,
        "time": 13424654474215,//100000 s later 
        "accuracy": 26
    },
     {//5
         "latitude": 49.0423046,//lat +1 
         "longitude": 14.4875852,
         "time": 13425654474215,//100000 s later 
         "accuracy": 26
     }

];

for (var recIdx = 0; recIdx < points.length; recIdx++) {
    var rec = points[recIdx];
    var qrec = store.newRecord({
        User: 1,
        Time: rec.time,
        Location: [rec.latitude, rec.longitude],
        Accuracy: rec.accuracy
    });
    aggr.onAdd(qrec);
    state = aggr.saveStateJson();
    console.log(state);
}
*/
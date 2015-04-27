/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
 
console.log(__filename)
var assert = require('../../src/nodejs/scripts/assert.js'); //adds assert.run function
var qm = require('qminer');
var fs = qm.fs;
// test feature construction
var analytics = qm.analytics;


describe('Time series test, old', function () {
	it('should survive', function () {
		
qm.delLock();

qm.config('qm.conf', true, 8080, 1024);
// add store.addTrigger method
var backward = require('../../src/nodejs/scripts/backward.js');
backward.addToProcess(process); // adds process.isArg function

var base = qm.create('qm.conf', "", true); // 2nd arg: empty schema, 3rd arg: clear db folder = true

console.log("TimeSeries", "Starting test based on time series sample");

// only report failours
assert.silent = !process.isArg("-verbose");
// name of the debug process
assert.consoleTitle = "TimeSeries";

base.createStore([{
    "name": "TimeSeries",
    "fields": [
      { "name": "Time", "type": "datetime" },
      { "name": "Value", "type": "float" }
    ],
    "joins": [],
    "keys": []
},
  {
      "name": "Resampled",
      "fields": [
        { "name": "Time", "type": "datetime" },
        { "name": "Value", "type": "float" },
        { "name": "Ema", "type": "float", "null": true }
      ],
      "joins": [],
      "keys": []
  }]);

// test empty stores
var TimeSeries = base.store("TimeSeries");
assert.equal(TimeSeries.name, "TimeSeries", "TimeSeries.name");
assert.ok(TimeSeries.empty, "TimeSeries.empty");
assert.equal(TimeSeries.length, 0, "TimeSeries.length");
assert.equal(TimeSeries.recs.length, 0, "TimeSeries.recs.length");
assert.equal(TimeSeries.fields.length, 2, "TimeSeries.fields.length");
assert.equal(TimeSeries.joins.length, 0, "TimeSeries.joins.length");
assert.equal(TimeSeries.keys.length, 0, "TimeSeries.keys.length");
var Resampled = base.store("Resampled");
assert.equal(Resampled.name, "Resampled", "Resampled.name");
assert.ok(Resampled.empty, "Resampled.empty");
assert.equal(Resampled.length, 0, "Resampled.length");
assert.equal(Resampled.recs.length, 0, "Resampled.recs.length");
assert.equal(Resampled.fields.length, 3, "Resampled.fields.length");
assert.equal(Resampled.joins.length, 0, "Resampled.joins.length");
assert.equal(Resampled.keys.length, 0, "Resampled.keys.length");

// insert triggers
var TimeSeriesAdd = 0;
TimeSeries.addTrigger({
    onAdd: function (val) {
        assert.exists(val, "onAdd: val");
        assert.exists(val.Time, "onAdd: val.Time");
        assert.exists(val.Value, "onAdd: val.Value");
        assert.exists(TimeSeries.getStreamAggr("tick").val, 'TimeSeries.getStreamAggr("tick").val');
        assert.exists(TimeSeries.getStreamAggr("emaTick").val, 'TimeSeries.getStreamAggr("emaTick").val');
        TimeSeriesAdd = TimeSeriesAdd + 1;
    }
});

var ResampledAdd = 0;
Resampled.addTrigger({
    onAdd: function (val) {
        assert.exists(val, "onAdd: val");
        assert.exists(val.Time, "onAdd: val.Time");
        assert.exists(val.Value, "onAdd: val.Value");
        assert.exists(Resampled.getStreamAggr("tick").val, 'Resampled.getStreamAggr("tick").val');
        assert.exists(Resampled.getStreamAggr("emaTick").val, 'Resampled.getStreamAggr("emaTick").val');
        Resampled.add({ $id: val.$id, Ema: Resampled.getStreamAggr("emaTick").val.Val });
        //1: val.Ema = Resampled.getStreamAggr("emaTick").val.Val;
        //2: val.Ema = Resampled.streamAggr.emaTick.val.Val;
        ResampledAdd = ResampledAdd + 1;
    }
});

// insert TimeSeries store aggregates
TimeSeries.addStreamAggr({
    name: "tick", type: "timeSeriesTick",
    timestamp: "Time", value: "Value"
});
TimeSeries.addStreamAggr({
    name: "emaTick", type: "ema",
    inAggr: "tick", emaType: "previous", interval: 60 * 1000, initWindow: 10 * 1000
});
// insert resampler
TimeSeries.addStreamAggr({
    name: "Resample10second", type: "resampler",
    outStore: "Resampled", timestamp: "Time",
    fields: [{ name: "Value", interpolator: "previous" }],
    createStore: false, interval: 10 * 1000
});
// insert Resampled store aggregates
Resampled.addStreamAggr({
    name: "tick", type: "timeSeriesTick",
    timestamp: "Time", value: "Value"
});
Resampled.addStreamAggr({
    name: "emaTick", type: "ema",
    inAggr: "tick", emaType: "previous", interval: 60 * 1000, initWindow: 10 * 1000
});

// insert a meassurement
// 1.26946,2012-01-08T22:00:14.153
assert.equal(TimeSeries.add({ "Time": "2012-01-08T22:00:14.153", "Value": 1.26946 }), 0, "TimeSeries.add");
assert.equal(TimeSeries.length, 1, "TimeSeries.length");
assert.exists(TimeSeries[0], "TimeSeries[0]");
assert.equal(TimeSeries[0].Time.toJSON(), "2012-01-08T22:00:14.153Z", "TimeSeries[0].Time.toJSON()");
assert.equal(TimeSeries[0].Value, 1.26946, "TimeSeries[0].Value");
// insert another measurement
// 1.26947,2012-01-08T22:00:14.497
assert.equal(TimeSeries.add({ "Time": "2012-01-08T22:00:14.497", "Value": 1.26947 }), 1, "TimeSeries.add");
assert.equal(TimeSeries.length, 2, "TimeSeries.length");
assert.exists(TimeSeries[1], "TimeSeries[1]");
assert.equal(TimeSeries[1].Time.toJSON(), "2012-01-08T22:00:14.497Z", "TimeSeries[1].Time.toJSON()");
assert.equal(TimeSeries[1].Value, 1.26947, "TimeSeries[1].Value");


// load from file
var fin = fs.openRead("./sandbox/timeseries/series.csv");
//TODO parse headers
var header = fin.readLine();
while (!fin.eof) {
    var line = fin.readLine();
    if (line == "") { continue; }
    try {
        var vals = line.split(',');
        var rec = { "Time": vals[1], "Value": parseFloat(vals[0]) };
        var length = TimeSeries.length;
        assert.equal(TimeSeries.add(rec), length, "TimeSeries.add(rec)");
        assert.exists(TimeSeries[length], "TimeSeries[length]");
    } catch (err) {
        console.say("TimeSeries", err);
    }
}

//// new feature space
var ftrSpace = new qm.FeatureSpace(base, [
	{ type: "numeric", source: "Resampled", field: "Value" },
	{ type: "numeric", source: "Resampled", field: "Ema" }
]);
assert.exists(ftrSpace, "ftrSpace");

for (var i = 0; i < Resampled.length; i++) {
    var rec = Resampled[i];
    var vec = ftrSpace.ftrVec(rec);
    assert.exists(vec, "ftrSpace.ftrVec(rec)");
}

base.close();
})});
var assert = require('assert.js');

console.say("TimeSeries", "Starting test based on time series sample");

// only report failours
assert.silent = process.isArg("-nopass");
// name of the debug process
assert.consoleTitle = "TimeSeries";

qm.createStore([{
    "name": "TimeSeries", 
    "fields": [ 
      { "name": "Time", "type": "datetime" },
      { "name": "Value", "type": "float" }
    ], 
    "joins": [ ],
    "keys": [ ]
  },
  {
    "name": "Resampled", 
    "fields": [ 
      { "name": "Time", "type": "datetime" },
      { "name": "Value", "type": "float" },
      { "name": "Ema", "type": "float", "null": true }
    ], 
    "joins": [ ],
    "keys": [ ]
  }]);
  
// test empty stores
var TimeSeries = qm.store("TimeSeries");
assert.equals(TimeSeries.name, "TimeSeries", "TimeSeries.name");
assert.ok(TimeSeries.empty, "TimeSeries.empty");
assert.equals(TimeSeries.length, 0, "TimeSeries.length");
assert.equals(TimeSeries.recs.length, 0, "TimeSeries.recs.length");
assert.equals(TimeSeries.fields.length, 2, "TimeSeries.fields.length");
assert.equals(TimeSeries.joins.length, 0, "TimeSeries.joins.length");
assert.equals(TimeSeries.keys.length, 0, "TimeSeries.keys.length");
var Resampled = qm.store("Resampled");
assert.equals(Resampled.name, "Resampled", "Resampled.name");
assert.ok(Resampled.empty, "Resampled.empty");
assert.equals(Resampled.length, 0, "Resampled.length");
assert.equals(Resampled.recs.length, 0, "Resampled.recs.length");
assert.equals(Resampled.fields.length, 3, "Resampled.fields.length");
assert.equals(Resampled.joins.length, 0, "Resampled.joins.length");
assert.equals(Resampled.keys.length, 0, "Resampled.keys.length");

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
		Resampled.add({ $id: val.$id, Ema: Resampled.getStreamAggr("emaTick").val.EMA });
		//1: val.Ema = Resampled.getStreamAggr("emaTick").val.EMA;
		//2: val.Ema = Resampled.streamAggr.emaTick.val.EMA;
		ResampledAdd = ResampledAdd + 1; 
	}
});

// insert TimeSeries store aggregates
TimeSeries.addStreamAggr({ name: "tick", type: "timeSeriesTick", 
	timestamp: "Time", value: "Value" });
TimeSeries.addStreamAggr({ name: "emaTick", type: "ema",
	inAggr: "tick", emaType: "previous", interval: 60*1000, initWindow: 10*1000 });
// insert resampler
TimeSeries.addStreamAggr({ name: "Resample10second", type: "resampler",
	outStore: "Resampled", timestamp: "Time", 
	fields: [ { name: "Value", interpolator: "previous" } ],
	createStore: false, interval: 10 * 1000 });
// insert Resampled store aggregates
Resampled.addStreamAggr({ name: "tick", type: "timeSeriesTick", 
	timestamp: "Time", value: "Value" });
Resampled.addStreamAggr({ name: "emaTick", type: "ema",
	inAggr: "tick", emaType: "previous", interval: 60*1000, initWindow: 10*1000 });

// insert a meassurement
// 1.26946,2012-01-08T22:00:14.153
assert.equals(TimeSeries.add({"Time": "2012-01-08T22:00:14.153", "Value": 1.26946}), 0, "TimeSeries.add");
assert.equals(TimeSeries.length, 1, "TimeSeries.length");
assert.exists(TimeSeries[0], "TimeSeries[0]");
assert.equals(TimeSeries[0].Time.string, "2012-01-08T22:00:14.153", "TimeSeries[0].Time.string");
assert.equals(TimeSeries[0].Value, 1.26946, "TimeSeries[0].Value");
// insert another measurement
// 1.26947,2012-01-08T22:00:14.497
assert.equals(TimeSeries.add({"Time": "2012-01-08T22:00:14.497", "Value": 1.26947}), 1, "TimeSeries.add");
assert.equals(TimeSeries.length, 2, "TimeSeries.length");
assert.exists(TimeSeries[1], "TimeSeries[1]");
assert.equals(TimeSeries[1].Time.string, "2012-01-08T22:00:14.497", "TimeSeries[1].Time.string");
assert.equals(TimeSeries[1].Value, 1.26947, "TimeSeries[1].Value");

// load from file
var fin =fs.openRead("./sandbox/timeseries/series.csv");
//TODO parse headers
var header = fin.getNextLn();
while (!fin.eof) {
	var line = fin.getNextLn();
	if (line == "") { continue; }
	try {
		var vals = line.split(',');
		var rec = { "Time": vals[1], "Value": parseFloat(vals[0]) };
		var length = TimeSeries.length;
		assert.equals(TimeSeries.add(rec), length, "TimeSeries.add(rec)");
		assert.exists(TimeSeries[length], "TimeSeries[length]");
	} catch (err) { 
		console.say("TimeSeries", err);
	}
}

// test feature construction
var analytics = require('analytics');
// new feature space
var ftrSpace = analytics.newFeatureSpace([
	{ type: "numeric", source: "Resampled", field: "Value" },
	{ type: "numeric", source: "Resampled", field: "Ema" }
]);
assert.exists(ftrSpace, "ftrSpace");

for (var i = 0; i < Resampled.length; i++) {
	var rec = Resampled[i];
	var vec = ftrSpace.ftrVec(rec);
	assert.exists(vec, "ftrSpace.ftrVec(rec)");
}

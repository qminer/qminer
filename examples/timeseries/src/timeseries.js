// Import analytics module
var analytics = require('analytics.js');

// Prepare shortcuts to raw time series and resampled store
var Raw = qm.store("Raw");
var Resampled = qm.store("Resampled");

// Initialize resamper from Raw to Resampled store. This results in
// in an equaly spaced time series with 10 second interval.
Raw.addStreamAggr({ name: "Resample10second", type: "resampler",
    outStore: "Resampled", timestamp: "Time", 
    fields: [ { name: "Value", interpolator: "previous" } ],
    createStore: false, interval: 10*1000 });
// Initialize stream aggregates on Resampled store for computing
// 1 minute and 10 minute exponential moving averages.
Resampled.addStreamAggr({ name: "tick", type: "timeSeriesTick", 
    timestamp: "Time", value: "Value" });
Resampled.addStreamAggr({ name: "ema1m", type: "ema",
    inAggr: "tick", emaType: "previous", interval: 60*1000, initWindow: 10*1000 });
Resampled.addStreamAggr({ name: "ema10m", type: "ema",
    inAggr: "tick", emaType: "previous", interval: 600*1000, initWindow: 10*1000 });
// Add a chain of 5 EMA stream aggregates and attach them to "tick" 
Resampled.addStreamAggr({ type: "itEma",
    numIter: 5, tmInterval: 10000, initMinMSecs: 0, inAggr: "tick", prefix: "itEma10s"});
// Buffer for keeping track of the record from 1 minute ago (6 records
// behind, which equals 60 seconds since Resampled store is equally spaced
// with 10 second rate).
Resampled.addStreamAggr({ name: "delay", type: "recordBuffer", size: 6});

// Declare features from the resampled timeseries which we will use
// to train the recursive linear regression.
var ftrSpace = analytics.newFeatureSpace([
    { type: "numeric", source: "Resampled", field: "Value" },
    { type: "numeric", source: "Resampled", field: "Ema1" },
    { type: "numeric", source: "Resampled", field: "Ema2" },
    { type: "multinomial", source: "Resampled", field: "Time", datetime: true }
]);
console.log("Feature space has " + ftrSpace.dim + " dimensions");

// Initialize linear regression model.
var linreg = analytics.newRecLinReg({ "dim": ftrSpace.dim, "forgetFact": 1.0 });

// We register a trigger to Resampled store, which takes the latest record
// and updates the recursive linear regression model.
Resampled.addTrigger({
    onAdd: function (val) {
        // Get the latest value for EMAs and store them along the 
        // record in the Resampled store.
        val.Ema1 = Resampled.getStreamAggr("ema1m").EMA;
        val.Ema2 = Resampled.getStreamAggr("ema10m").EMA;
        // See what would the current model would predict given
        // the new record, and store this for evaluation later on.
        val.Prediction = linreg.predict(ftrSpace.ftrVec(val))
        // Get the id of the record from a minute ago.
        var trainRecId = Resampled.getStreamAggr("delay").last;
        // Update the model, once we have at leats 1 minute worth of data
        if (trainRecId > 0) { linreg.learn(ftrSpace.ftrVec(Resampled[trainRecId]), val.Value); }
        // Get the current value and compare against prediction for a minute ago
        var diff = val.Value - Resampled[trainRecId].Prediction;
        console.log("Diff: " + diff + ", Value: " + val.Value + ", Prediction: " + Resampled[trainRecId].Prediction);
    }
});

// Load training data from CSV file.
var fin = fs.openRead("./sandbox/timeseries/series.csv");
var header = fin.readLine(); var lines = 0;
while (!fin.eof) {
    lines = lines + 1;
    if (lines % 1000 == 0) { console.log("Loaded: " + lines); }
    var line = fin.readLine();
    if (line == "") { continue; }
    try {
        var vals = line.split(',');
        var rec = { "Time": vals[1], "Value": parseFloat(vals[0]) };
        Raw.add(rec);
    } catch (err) { 
        console.say("Raw", err);
    }
}


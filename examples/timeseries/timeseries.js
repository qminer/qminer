// Import main modules
var qm = require('qminer');
var loader = require('qminer-data-loader');
var fs = qm.fs;
var analytics = qm.analytics;

var base = new qm.Base({
	mode: "createClean",
	schema: [
	    {
	      "name": "Raw",
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
	        { "name": "Ema1", "type": "float", "null": true },
	        { "name": "Ema2", "type": "float", "null": true },
	        { "name": "Prediction", "type": "float", "null": true }
	      ],
	      "joins": [
	        { "name": "source", "type": "field", "store": "Raw" }
	      ],
	      "keys": [ ]
	    }
	]
});

// Prepare shortcuts to raw time series and resampled store
var Raw = base.store("Raw");
var Resampled = base.store("Resampled");

// Initialize resampler from Raw to Resampled store. This results
// in an equally spaced time series with 10 second interval.
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
// Buffer for keeping track of the record from 1 minute ago (6 records
// behind, which equals 60 seconds since Resampled store is equally spaced
// with 10 second rate).
Resampled.addStreamAggr({ name: "delay", type: "recordBuffer", size: 6});

// Declare features from the resampled timeseries which we will use
// to train the recursive linear regression.
var ftrSpace = new qm.FeatureSpace(base, [
    { type: "numeric", source: "Resampled", field: "Value" },
    { type: "numeric", source: "Resampled", field: "Ema1" },
    { type: "numeric", source: "Resampled", field: "Ema2" },
    { type: "multinomial", source: "Resampled", field: "Time", datetime: true }
]);
console.log("Feature space has " + ftrSpace.dim + " dimensions");

// Initialize linear regression model.
var linreg = new analytics.RecLinReg({ "dim": ftrSpace.dim, "forgetFact": 1.0 });

// We register a trigger to Resampled store, which takes the latest record
// and updates the recursive linear regression model.
Resampled.addTrigger({
    onAdd: function (val) {
        // Get the latest value for EMAs and store them along the
        // record in the Resampled store.
		val.Ema1 = base.getStreamAggr("ema1m").val.Val;
        val.Ema2 = base.getStreamAggr("ema10m").val.Val;		
        // See what the current model would predict given
        // the new record, and store this for evaluation later on.
        val.Prediction = linreg.predict(ftrSpace.extractVector(val));
        // Get the id of the record from a minute ago.
        var trainRecId = base.getStreamAggr("delay").val.oldest.$id;
        //// Update the model, once we have at leats 1 minute worth of data
		if (trainRecId > 0) { linreg.partialFit(ftrSpace.extractVector(Resampled[trainRecId]), val.Value); }
        // Get the current value and compare against prediction for a minute ago
        var diff = val.Value - Resampled[trainRecId].Prediction;
        console.log("Diff: " + diff + ", Value: " + val.Value + ", Prediction: " + Resampled[trainRecId].Prediction);
    }
});

loader.loadForexDataset(Raw);

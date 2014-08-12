// import libraries
analytics = require('analytics.js');
time = require('time.js');
viz = require('visualization.js');

// simple time series store
var StoresDef = [
  {
      name: "ts", 
      fields: [ 
        { name: "Time", type: "datetime" },
        { name: "Val", type: "float" }
      ], 
  },
]
// create store
qm.createStore(StoresDef);
var ts = qm.store('ts');

// create three stream aggregates
// s1  : rec.Val + noise
s1 = qm.newStreamAggr(new function () {
    this.name = 'js';
    var data = 0;
    var time;
    var longtime;
    this.onAdd = function (rec) {
        longtime = rec.Time.windowstimestamp;
        time = rec.Time.string;
        data = rec.Val + 0.5*Math.random();
    };
    this.saveJson = function (limit) {
        return { Val: data, Time: time };
    };
    this.getFlt = function () {
        return data;
    }
    this.getTm = function () {
        return longtime;
    }
}, "ts");
// s2 : exponential moving average, takes tick as input
s2 = qm.newStreamAggr({ name: "ema", type: "ema", inAggr: "js", emaType: "linear", interval: 5 * 1000 }, "ts");

// stores stream aggregate values
var buffer = [];

// generate sine time series with period of 1 minute
var tm = time.now;
var tmshift = tm.clone();
var periodInSeconds = 60;
// number of steps = number of seconds
for (var step = 0; step < 180; step++) {
    tmshift.add(1, 'second');
    var valshift = Math.sin(2 * Math.PI * tmshift.timestamp / periodInSeconds);
    ts.add(ts.newRec({ Time: tmshift.string, Val: valshift }));
    // print json object of the most recen measurement
    printj(ts.last);
    // get current stream aggregate state and and add it in the buffer (as a json)
    buffer.push(qm.getAllStreamAggrVals(ts));
}

//// Convert to highcharts compatible data format
//series: [{
//    name: 'Winter 2007-2008',
//    // Define the data points. All series have a dummy year
//    // of 1970/71 in order to be compared on the same x axis. Note
//    // that in JavaScript, months start at 0 for January, 1 for February etc.
//    data: [
//        [Date.UTC(1970, 9, 27), 0],
//        [Date.UTC(1970, 10, 10), 0.6],

//    ]
//}, {
//    name: 'Winter 2008-2009',
//    data: [
//        [Date.UTC(1970, 9, 18), 0],
//        [Date.UTC(1970, 9, 26), 0.2],

//    ]
//}]
viz.drawHighChartsTimeSeries(viz.highchartsConverter(buffer), "plot.html", { title: { text: "js(t) = sin(t) + noise, ema(t) = EMA(js(t))" } });

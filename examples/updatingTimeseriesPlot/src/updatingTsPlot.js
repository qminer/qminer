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
    this.name = 'timeseries';
    var data = 0;
    var time;
    var longtime;
    this.onAdd = function (rec) {
        longtime = rec.Time.windowsTimestamp;
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
s2 = qm.newStreamAggr({ name: "ema", type: "ema", inAggr: "timeseries", emaType: "linear", interval: 5 * 1000 }, "ts");

// stores stream aggregate values
var buffer = [];
var bufferSize = 120; 

// Offline (no server is needed)
//viz.drawHighChartsTimeSeries(viz.highchartsTSConverter(buffer), "plot.html", { title: { text: "js(t) = sin(t) + noise, ema(t) = EMA(js(t))" } });
    
// Online (server must be running)
////client page with the visualization: http://localhost:8080/www/client.html
////serve data at: http://localhost:8080/updatingTsPlot/data
http.onGet("data", function (req, resp) {
    //return http.jsonp(req, resp, viz.highchartsTSConverter(buffer[buffer.length - 1]));
    // Send only current value
    return http.jsonp(req, resp, viz.highchartsTSConverter([qm.getAllStreamAggrVals(ts)]));
})

http://localhost:8080/updatingTsPlot/histData
http.onGet("histData", function (req, resp) {
    //return http.jsonp(req, resp, viz.highchartsTSConverter(buffer[buffer.length - 1]));
    // Send only current value
    return http.jsonp(req, resp, viz.highchartsTSConverter(buffer));
})

http://localhost:8080/updatingTsPlot/debug
http.onGet("debug", function (req, resp) {
    console.log("Debug mode...");
    eval(breakpoint);
    return http.jsonp(req, resp, "Debug mode...");
})

////send data to: http://localhost:8080/updatingTsPlot/import
http.onGet("import", function (req, resp) {
    //printj(req)
    try {
        var rec = JSON.parse(req.args.data);
        printj(rec)
        ts.add(ts.newRec(rec))

        if (buffer.length >= bufferSize) buffer.shift();
        buffer.push(qm.getAllStreamAggrVals(ts));

        resp.setStatusCode(200);
        return http.jsonp(req, resp, "OK");

    } catch (err) {
        console.log("Error", err);

        resp.setStatusCode(404);
        return http.jsonp(req, resp, "Error");
    }
})
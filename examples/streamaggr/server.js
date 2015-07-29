///////////////////////////////////////////////////////////
// Creation of the store

// import qm module
var qm = require('qminer');
// create a base with two stores
var base = new qm.Base({
    mode: 'createClean',
    schema: [
        // used for the the Stat/Smooth chart
        {
            name: 'Stats',
            fields: [
                { name: 'Value', type: 'float' },
                { name: 'Time', type: 'datetime' }
            ]
        },
    {
        name: 'Average',
        fields: [
            { name: 'Value', type: 'float' },
            { name: 'Time', type: 'datetime' }
        ]
    },
    // used for the resampler chart
    {
        name: 'Random',
        fields: [
            { name: 'Value', type: 'float' },
            { name: 'Time', type: 'datetime' }
        ]
    },
    {
        name: 'Resampler',
        fields: [
            { name: 'Value', type: 'float' },
            { name: 'Time', type: 'datetime' }
        ]
    },
    // used for the merger charts
    {
        name: 'Gauss',
        fields: [
            { name: 'Value', type: 'float' },
            { name: 'Time', type: 'datetime' }
        ]
    },
    {
        name: 'OtherGauss',
        fields: [
            { name: 'Value', type: 'float' },
            { name: 'Time', type: 'datetime' }
        ]
    },
    {
        name: 'Merger',
        fields: [
            { name: 'Gaussian', type: 'float' },
            { name: 'OtherGaussian', type: 'float' },
            { name: 'Time', type: 'datetime' }
        ]
    }
    ]
});
///////////////////////////////////////////////////////////
// Agregators

// add a time series window stream aggregator for the 'Value' field in the 'Stats' store
var ts = {
    name: 'StatWindow',
    type: 'timeSeriesWinBuf',
    store: 'Stats',
    timestamp: 'Time',
    value: 'Value',
    winsize: 10 * 1000
};
var tsWindow = base.store('Stats').addStreamAggr(ts);

// add a moving average stream aggregator connected to the 'StatWindow' time series
var ma = {
    name: 'StatAverage',
    type: 'ma',
    store: 'Stats',
    inAggr: 'StatWindow'
};
var maAverage = base.store('Stats').addStreamAggr(ma);

// add a resampler stream aggregator connected to the 'Random' store
var res = {
    name: 'ResamplerAggr',
    type: 'resampler',
    store: 'Random',
    outStore: 'Resampler',
    timestamp: 'Time',
    fields: [
        { name: 'Value', interpolator: 'previous' }
    ],
    createStore: false,
    interval: 1000
}
var resampler = base.store('Random').addStreamAggr(res);

// add a merger stream aggregator connected to the 'Gauss' and 'OtherGauss' stores
var mer = {
    name: 'MergerAggr',
    type: 'stmerger',
    outStore: 'Merger',
    createStore: false,
    timestamp: 'Time',
    fields: [
        { source: 'Gauss', inField: 'Value', outField: 'Gaussian', interpolation: 'linear', timestamp: 'Time' },
        { source: 'OtherGauss', inField: 'Value', outField: 'OtherGaussian', interpolation: 'linear', timestamp: 'Time' }
    ]
};
var merger = new qm.StreamAggr(base, mer);

///////////////////////////////////////////////////////////
// Getting the values

// import the os module
var os = require('os');

var dataStats = function () {
    var date = new Date().toISOString();
    var lastStat = base.store('Stats').last;
    if (lastStat != null) {
        base.store('Stats').push({ Value: lastStat.Value + qm.la.randn(), Time: date });
    } else {
        base.store('Stats').push({ Value: qm.la.randn(), Time: date });
    }
    base.store('Average').push({ Value: maAverage.getFloat(), Time: date });
    setTimeout(dataStats, 1000);
};

var dataResampler = function () {
    var date = new Date().toISOString();
    var lastRandom = base.store('Random').last;
    if (Math.random() > 0.5) {
        if (lastRandom != null) {
            base.store('Random').push({ Value: lastRandom.Value + qm.la.randn(), Time: date });
        } else {
            base.store('Random').push({ Value: qm.la.randn(), Time: date });
        }
    }
    setTimeout(dataResampler, 1000);
}

// counters for the merger aggregator
var numberOfNewPoints = 0;
var dataGauss = function () {
    var date = new Date().toISOString();
    var numBefore= base.store('Merger').length;

    // first store: Gauss
    if (Math.random() > 0.5) { base.store('Gauss').push({ Value: qm.la.randn(), Time: date }); };
    // second store: otherGauss
    if (Math.random() > 0.5) { base.store('OtherGauss').push({ Value: qm.la.randn(), Time: date }); };

    numberOfNewPoints = base.store('Merger').length - numBefore;
    setTimeout(dataGauss, 1000);
}

///////////////////////////////////////////////////////////
// Server

dataStats();
dataResampler();
dataGauss();
// create the server
var path = require('path');
var express = require('express');
var app = express();
var http = require('http').Server(app);
var io = require('socket.io')(http);

// adding the css files (or static files) to the server
app.use(express.static(path.join(__dirname + '/public')));

// when the client calls the server, it calls the dataSystemStats() and the example.html
app.get('/', function (request, response) {
    response.sendFile(__dirname + '/example.html');
});

io.on('connection', function (socket) {

    console.log('a user connected');
    socket.on('disconnect', function () {
        console.log('user disconnected');
    });

    setInterval(function () {
        // used for sending the moving average values
        socket.emit('getStats',
            {
                stat: base.store("Stats").last.Value,
                smooth: base.store("Average").last.Value,
                timeStat: base.store("Stats").last.Time
            }
        )
    }, 1000);

    setInterval(function () {
        // used for sending the resampler values
        var resValue = [],
            resTime = [],
            last = 0,
            previous = 0;
        if (base.store("Random").length > 1) {
            last = (new Date(base.store("Random").last.Time)).getTime(),
            previous = (new Date(base.store("Random")[base.store("Random").length - 2].Time)).getTime();
        } else if (base.store("Random").length > 0){
            previous = (new Date(base.store("Random").first.Time)).getTime();
        }
        if ((last - previous) >= 1000) {
            for (var i = Math.floor((last - previous) / 1000) ; i > 0; i--) {
                resValue.push(base.store("Resampler")[base.store("Resampler").length - i].Value);
                resTime.push(base.store("Resampler")[base.store("Resampler").length - i].Time);
            }
        }
        // used for calculating how many points from resampler we need
        var numberOfPoints = 0;
        if (base.store("Random").length > 10) {
            numberOfPoints = (new Date(base.store("Random")[base.store("Random").length - 10].Time)).getTime();
   
        } else if (base.store("Random").length > 0) {
            numberOfPoints = (new Date(base.store("Random").first.Time)).getTime();
        }
        numberOfPoints = Math.floor((last - numberOfPoints) / 1000);
        if (base.store("Random").last != null) {
            socket.emit('getResampler',
                {
                    random: base.store("Random").last.Value,
                    timeRes: base.store("Random").last.Time,
                    resValue: resValue,
                    resTime: resTime,
                    number: numberOfPoints
                }
            );
        }
    }, 1000);


    setInterval(function () {
        if (base.store('Gauss').last != null) {
            socket.emit('getMergerGauss',
                {
                    gaussVal: base.store('Gauss').last.Value,
                    gaussTime: base.store('Gauss').last.Time,
                });
        }
        if (base.store('OtherGauss').last != null) {
            socket.emit('getMergerOtherGauss',
                {
                    otherGaussVal: base.store('OtherGauss').last.Value,
                    otherGaussTime: base.store('OtherGauss').last.Time
                });
        }
    }, 1000);

    setInterval(function () {
        if (base.store('Merger').last != null) {
            var firstGauss = [], secondGauss = [], mergerTime = [];
            for (var i = numberOfNewPoints; i > 0; i--) {
                var k = base.store('Merger').length - i;
                firstGauss.push(base.store('Merger')[k].Gaussian);
                secondGauss.push(base.store('Merger')[k].OtherGaussian);
                mergerTime.push(base.store('Merger')[k].Time);
            }
            socket.emit('getMerger',
                {
                    firstGauss: firstGauss,
                    secondGauss: secondGauss,
                    mergerTime: mergerTime
                }
            );
        };
    }, 1000);

});

// creates the server listener, so that we know that it's connected
http.listen(3000, '127.0.0.1');
console.log('Server running at http://127.0.0.1:3000/');

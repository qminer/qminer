///////////////////////////////////////////////////////////
// Creation of the store

// import qm module
var qm = require('qminer');
// create a base with two stores:   'SystemStats' saves the CPU and free memory
//                                  'AverageValues' saves the average CPU and free memory
var base = new qm.Base({
    mode: 'createClean',
    schema: [
        // used for the the Stat/Smooth
        {
        name: 'SystemStats',
        fields: [
            { name: 'Delta', type: 'float' },
            { name: 'Time', type: 'datetime' }
        ]
    },
    {
        name: 'AverageValues',
        fields: [
            { name: 'Delta', type: 'float' },
            { name: 'Time', type: 'datetime' }
        ]
    }
    ]
});
///////////////////////////////////////////////////////////
// Agregators

// add a time series window stream aggregator for the 'Delta' field in the 'SystemStats' store
ts = {
    name: 'MemWindow',
    type: 'timeSeriesWinBuf',
    store: 'SystemStats',
    timestamp: 'Time',
    value: 'Delta',
    winsize: 10 * 1000
};
var tsMemory = base.store('SystemStats').addStreamAggr(ts);

// add a moving average stream aggregator connected to the 'MemWindow' time series
ma = {
    name: 'DelAverage',
    type: 'ma',
    store: 'SystemStats',
    inAggr: 'MemWindow'
};
var maDelta = base.store('SystemStats').addStreamAggr(ma);

///////////////////////////////////////////////////////////
// Getting the values

// import the os module
var os = require('os');

var dataSystemStats = function () {
    var date = new Date().toISOString();

    base.store('SystemStats').push({ Delta: 100 * Math.random(), Time: date });
    base.store('AverageValues').push({ Delta: maDelta.getFloat(), Time: date });
    setTimeout(dataSystemStats, 1000);
};

///////////////////////////////////////////////////////////
// Server

dataSystemStats();
// create the server
var path = require('path');
var express = require('express');
var app = express();
var http = require('http').Server(app);
var io = require('socket.io')(http);

// adding the css files (or static files) to the server
app.use(express.static(path.join(__dirname + '/public')))

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
        socket.emit('getValues',
            {
                stat: base.store("SystemStats").last.Delta,
                smooth: base.store("AverageValues").last.Delta,
                now: base.store("SystemStats").last.Time
            }
        )
    }, 1000);
});

// creates the server listener, so that we know that it's connected
http.listen(3000, '127.0.0.1');
console.log('Server running at http://127.0.0.1:3000/');

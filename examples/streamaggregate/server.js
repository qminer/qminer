///////////////////////////////////////////////////////////
// Creation of the store

// import qm module
var qm = require('qminer');
// create a base with two stores

var saBase = new qm.Base({
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
            { name: 'FirstValue', type: 'float' },
            { name: 'SecondValue', type: 'float' },
            { name: 'Time', type: 'datetime' }
        ]
    }
    ]
});

///////////////////////////////////////////////////////////
// Stream Agregates

// add a time series window stream aggregate for the 'Value' field in the 'Stats' store
var ts = {
    name: 'StatWindow',
    type: 'timeSeriesWinBuf',
    store: 'Stats',
    timestamp: 'Time',
    value: 'Value',
    winsize: 10 * 1000
};
var tsWindow = saBase.store('Stats').addStreamAggr(ts);

// add a moving average stream aggregate connected to the 'StatWindow' time series
var ma = {
    name: 'StatAverage',
    type: 'ma',
    store: 'Stats',
    inAggr: 'StatWindow'
};
var maAverage = saBase.store('Stats').addStreamAggr(ma);

// add a resampler stream aggregate connected to the 'Random' store
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
var resampler = saBase.store('Random').addStreamAggr(res);

// add a merger stream aggregate connected to the 'Gauss' and 'OtherGauss' stores
var mer = {
    name: 'MergerAggr',
    type: 'merger',
    outStore: 'Merger',
    createStore: false,
    timestamp: 'Time',
    fields: [
        { source: 'Gauss', inField: 'Value', outField: 'FirstValue', interpolation: 'linear', timestamp: 'Time' },
        { source: 'OtherGauss', inField: 'Value', outField: 'SecondValue', interpolation: 'linear', timestamp: 'Time' }
    ]
};
var merger = new qm.StreamAggr(saBase, mer);


///////////////////////////////////////////////////////////
// Getting the values

// putting the Brownian motion to the store for the Stat/Average
var dataStats = function () {
    var date = new Date().toISOString();
    var lastStat = saBase.store('Stats').last;
    if (lastStat != null) { saBase.store('Stats').push({ Value: lastStat.Value + qm.la.randn(), Time: date }); }
    else { saBase.store('Stats').push({ Value: qm.la.randn(), Time: date }); }

    saBase.store('Average').push({ Value: maAverage.getFloat(), Time: date });
    setTimeout(dataStats, 1000);
};

// putting the Brownian motion to the store for Resampler
var dataResampler = function () {
    var date = new Date().toISOString();
    var lastRandom = saBase.store('Random').last;
    if (Math.random() > 0.5) {
        if (lastRandom != null) { saBase.store('Random').push({ Value: lastRandom.Value + qm.la.randn(), Time: date }); }
        else { saBase.store('Random').push({ Value: qm.la.randn(), Time: date }); }
    }
    setTimeout(dataResampler, 1000);
};

// putting the Guassian values in the store for Merger
var dataGauss = function () {
    var date = new Date().toISOString();
    var numBefore = saBase.store('Merger').length;
    // first store: Gauss
    if (Math.random() > 0.5) { saBase.store('Gauss').push({ Value: qm.la.randn(), Time: date }); };
    // second store: otherGauss
    if (Math.random() > 0.5) { saBase.store('OtherGauss').push({ Value: qm.la.randn(), Time: date }); };

    setTimeout(dataGauss, 1000);
};

// start putting records in the stores
dataStats();
dataResampler();
dataGauss();


///////////////////////////////////////////////////////////
// Server

// create the server
var path = require('path');
var express = require('express');
var app = express();
var http = require('http').Server(app);
var io = require('socket.io')(http);

// make server function
function makeServer() {
	// adding the css files (or static files) to the server
	app.use(express.static(path.join(__dirname + '/public')));

	// when the client calls the server, it calls the dataSystemStats() and the example.html
	app.get('/', function (request, response) {
	    response.sendFile(__dirname + '/streamaggregate.html');
	});

    	app.get('/moving_average.html', function (request, response) {
	    response.sendFile(__dirname + '/moving_average.html');
	});
	app.get('/resampler.html', function (request, response) {
	    response.sendFile(__dirname + '/resampler.html');
	});
	app.get('/merger.html', function (request, response) {
	    response.sendFile(__dirname + '/merger.html');
	});

	// creates the server listener, so that we know that it's connected
	var server = http.listen(3000, function () {
		var port = server.address().port;
		console.log('Server started at port %s', port);
	});
	return server;
};

// the callback function
var getData = function () {
    // create a new stream aggregate for the 'Stats' store
    new qm.StreamAggr(saBase, new function () {
        this.name = 'statsPush';
        this.onAdd = function (rec) {
            if (io.sockets.connected) {
                io.sockets.emit('getStats', {
                    value: rec.Value,
                    time: rec.Time
                });
            }
        }
        this.saveJson = function (limit) {
            return {};
        }
    }, 'Stats');
    // create a new stream aggregate for the 'Average' store
    new qm.StreamAggr(saBase, new function () {
        this.name = 'averagePush';
        this.onAdd = function (rec) {
            if (io.sockets.connected) {
                io.sockets.emit('getAverage', {
                    value: rec.Value,
                    time: rec.Time
                });
            }
        }
        this.saveJson = function (limit) {
            return {};
        }
    }, 'Average');
    // create a new stream aggregate for the 'Random' store
    new qm.StreamAggr(saBase, new function () {
        this.name = 'randomPush';
        this.onAdd = function (rec) {
            if (io.sockets.connected) {
                io.sockets.emit('getRandom', {
                    value: rec.Value,
                    time: rec.Time
                });
            }
        }
        this.saveJson = function (limit) {
            return {};
        }
    }, 'Random');
    // create a new stream aggregate for the 'Resampler' store TODO
    new qm.StreamAggr(saBase, new function () {
        this.name = 'resamplerPush';
        this.onAdd = function (rec) {
            if (io.sockets.connected) {
                io.sockets.emit('getResampler', {
                    value: rec.Value,
                    time: rec.Time
                });
            }
        }
        this.saveJson = function (limit) {
            return {};
        }
    }, 'Resampler');
    // create a new stream aggregate for the 'Gauss' store
    new qm.StreamAggr(saBase, new function () {
        this.name = 'gaussPush';
        this.onAdd = function (rec) {
            if (io.sockets.connected) {
                io.sockets.emit('getGauss', {
                    value: rec.Value,
                    time: rec.Time
                });
            }
        };
        this.saveJson = function (limit) {
            return {};
        };
    }, 'Gauss');
    // create a new stream aggregate for the 'OtherGauss' store
    new qm.StreamAggr(saBase, new function () {
        this.name = 'otherGaussPush';
        this.onAdd = function (rec) {
            if (io.sockets.connected) {
                io.sockets.emit('getOtherGauss', {
                    value: rec.Value,
                    time: rec.Time
                });
            }
        };
        this.saveJson = function (limit) {
            return {};
        };
    }, 'OtherGauss');
    // create a new stream aggregate for the 'Merger' store
    new qm.StreamAggr(saBase, new function () {
        this.name = 'mergerPush';
        this.onAdd = function (rec) {
            if (io.sockets.connected) {
                io.sockets.emit('getMerger', {
                    firstValue: rec.FirstValue,
                    secondValue: rec.SecondValue,
                    time: rec.Time
                });
            }
        };
        this.saveJson = function (limit) {
            return {};
        };
    }, 'Merger');
}

// create the stream aggregates that send the data to the client-side
getData();
// start server
makeServer();

// exports server
module.exports = makeServer;

// create the server
var path = require('path');
var express = require('express');
var app = express();
var http = require('http').Server(app);
var io = require('socket.io')(http);

// qminer backend function container
var qminerBackend = require('./qminerBackend');
var qmBackend = new qminerBackend();

// send data to qminer backend
setInterval(qmBackend.pushBrownianStats, 1000, qmBackend);
setInterval(qmBackend.pushBrownianResampler, 1000, qmBackend);
setInterval(qmBackend.pushGaussianMerger, 1000, qmBackend);

// make server function
function makeServer() {
	var pathDir = path.resolve(__dirname + `/../`);
	// adding the css files (or static files) to the server
	app.use(express.static(pathDir + '/public'));

	// when the client calls the server, it calls the dataSystemStats() and the example.html
	app.get('/', function (req, res) {
	    res.sendFile(pathDir + '/html/index.html');
	});
    app.get('/moving_average.html', function (req, res) {
	    res.sendFile(pathDir + '/html/moving_average.html');
	});
	app.get('/resampler.html', function (req, res) {
	    res.sendFile(pathDir + '/html/resampler.html');
	});
	app.get('/merger.html', function (req, res) {
	    res.sendFile(pathDir + '/html/merger.html');
	});

	// creates the server listener, so that we know that it's connected
	var server = http.listen(3000, function () {
		var port = server.address().port;
		console.log('Server started at port %s', port);
	});
	return server;
}

qmBackend.createIOStream(io);

// exports server
module.exports = makeServer;

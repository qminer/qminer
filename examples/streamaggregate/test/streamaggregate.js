var qminerBackend = require('../lib/qminerBackend');
var assert = require('assert');

var express = require('express');
var app = express();
var http = require('http').Server(app);
var io = require('socket.io')(http);

describe('Streamaggregate example test', function () {
    this.timeout(10000);
    var qmBackend;
    before(function () {
        qmBackend = new qminerBackend();
    });

    // close server after each test
    after(function () {
        qmBackend.base.close();
    });

    // test server
    it('shouldn\'t crash', function () {
        assert.doesNotThrow(function () {
            qmBackend.pushBrownianStats(qmBackend);
            qmBackend.pushBrownianResampler(qmBackend);
            qmBackend.pushGaussianMerger(qmBackend);
            qmBackend.createIOStream(io);
        });
    });
});
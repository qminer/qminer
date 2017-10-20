"use strict";

// import qm module
var qm = require('qminer');

class QMinerBackend {

    constructor() {
        // create base
        this._createBase();
        // add stream aggrs
        this._loadSteamAggr();
    }

    /**
     * Creates the QMiner base.
     */
    _createBase() {
        this.base = new qm.Base({
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
    }

    /**
     * Adds a stream aggr to the base.
     * @param {String} storeName - Store name.
     * @param {Object} opts - The stream aggr options.
     */
    _addStreamAggr(opts, storeName, singleStore=false) {
        if (singleStore) {
            this.base.store(storeName).addStreamAggr(opts);
        } else {
            new qm.StreamAggr(this.base, opts);
        }
    }

    /**
     * Loads stream aggregates.
     */
    _loadSteamAggr() {
        let streamAggrOpts = require('./streamAggrOpts');
        streamAggrOpts.singleStore.forEach(el => this._addStreamAggr(el.opts, el.store, true));
        streamAggrOpts.multiStore.forEach(el => this._addStreamAggr(el.opts));
    }

    /**
     * Push a random value to the store.
     * @param {String} storeName - Store name.
     * @param {String} date - The ISO string of a date.
     */
    _pushValueToStore(storeName, date) {
        let val = this.base.store(storeName).last;
        if (val != null) { this.base.store(storeName).push({ Value: val.Value + qm.la.randn(), Time: date }); }
        else { this.base.store(storeName).push({ Value: qm.la.randn(), Time: date }); }
    }

    /**
     * Push values from brownian motion to store.
     */
    pushBrownianStats(self) {
        let date = new Date().toISOString();
        self._pushValueToStore('Stats', date);
        // push average value
        let maAvg = self.base.getStreamAggr('StatAverage');
        self.base.store('Average').push({ Value: maAvg.getFloat(), Time: date });
    }

    /**
     * Push values from brownian motion to store.
     */
    pushBrownianResampler(self) {
        var date = new Date().toISOString();
        self._pushValueToStore('Random', date);
    }

    /**
     * Push random values for merger.
     */
    pushGaussianMerger(self) {
        var date = new Date().toISOString();
        var val = self.base.store('Merger').length;
        if (Math.random() > 0.5) { self.base.store('Gauss').push({ Value: val + qm.la.randn(), Time: date }); }
        if (Math.random() > 0.5) { self.base.store('OtherGauss').push({ Value: val + qm.la.randn(), Time: date }); }
    }


    /**
     * Add a new stream aggregate for sending data via socket.
     * @param {String} storeName - Store name.
     * @param {String} streamName - Stream name.
     * @param {Socket} io - The socket library.
     * @param {String} socketName - Socket name.
     * @param {Boolean} merger - Select socket object.
     */
    _addNewStreamAggr(storeName, streamName, io, socketName, merger=false) {
        // creates socket emit object
        function createSocketObj(rec, merger) {
            return merger ? {
                firstValue: rec.FirstValue,
                secondValue: rec.SecondValue,
                time: rec.Time
            } : {
                value: rec.Value,
                time: rec.Time
            };
        }

        new qm.StreamAggr(this.base, new function () {
            this.name = streamName;
            this.onAdd = function (rec) {
                if (io.sockets.connected) {
                    io.sockets.emit(socketName, createSocketObj(rec, merger));
                }
            };
            this.saveJson = function (limit) {
                return {};
            };
        }, storeName);
    }

    /**
     * Creates all the stream aggregates to send data via sockets.
     * @param {Socket} io - Socket.
     */
    createIOStream(io) {
        require('./streamAggrOpts').emitSocketOpts.forEach(el => {
            let mergerFlag = el.storeName === "Merger";
            this._addNewStreamAggr(el.storeName, el.streamName, io, el.socketName, mergerFlag);
        });
    }
}

module.exports = QMinerBackend;
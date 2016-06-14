/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

// console.log(__filename)
var assert = require('../../src/nodejs/scripts/assert.js'); //adds assert.run function
var qm = require('../../');

var verbose = false;

describe('Merger test, old', function () {
    it('should survive', function (done) {
        qm.delLock();
        var backward = require('../../src/nodejs/scripts/backward.js');
        backward.addToProcess(process); // adds process.isArg function

        var base = new qm.Base({ mode: 'createClean' });

        if (verbose) {
            console.log("Merger", "Testing merger");
        }
        // only report failours
        assert.silent = !process.isArg("-verbose");
        // name of the debug process
        assert.consoleTitle = "Merger";

        function findMinVal(idxs, vals) {
            var minTm = Number.MAX_VALUE;
            var minIdx = -1;

            for (var i = 0; i < idxs.length; i++) {
                if (idxs[i] < vals[i].length && vals[i][idxs[i]].timestamp < minTm) {
                    minTm = vals[i][idxs[i]].timestamp;
                    minIdx = i;
                }
            }

            return minIdx;
        }

        function testMerger() {
            if (verbose) {
                console.log('Testing merger...');
                console.log('TEST 1');
            }
            base.createStore([
               {
                   "name": "sig1",
                   "fields": [
                       { "name": "time_ms", "type": "uint64" },
                       { "name": "time", "type": "datetime" },
                       { "name": "value", "type": "float" }
                   ]
               },
               {
                   "name": "sig2",
                   "fields": [
                       { "name": "time_ms", "type": "uint64" },
                       { "name": "time", "type": "datetime" },
                       { "name": "value", "type": "float" }
                   ]
               },
               {
                   "name": "sig3",
                   "fields": [
                       { "name": "time_ms", "type": "uint64" },
                       { "name": "time", "type": "datetime" },
                       { "name": "value", "type": "float" }
                   ]
               },
               {
                   "name": "sig4",
                   "fields": [
                       { "name": "time_ms", "type": "uint64" },
                       { "name": "time", "type": "datetime" },
                       { "name": "value", "type": "float" }
                   ]
               },
               {
                   "name": "joined",
                   "fields": [
                       { "name": "time", "type": "datetime" },
                       { "name": "sig1", "type": "float" },
                       { "name": "sig2", "type": "float" },
                       { "name": "sig3", "type": "float" },
                       { "name": "sig4", "type": "float" }
                   ]
               }
            ]);

            new qm.StreamAggr(base, {
                type: 'merger',
                name: 'blabla',
                outStore: 'joined',
                createStore: false,
                timestamp: 'time',
                fields: [
                    { source: 'sig1', inField: 'value', outField: 'sig1', interpolation: 'linear', timestamp: 'time' },
                    { source: 'sig2', inField: 'value', outField: 'sig2', interpolation: 'linear', timestamp: 'time' },
                    { source: 'sig3', inField: 'value', outField: 'sig3', interpolation: 'linear', timestamp: 'time' },
                    { source: 'sig4', inField: 'value', outField: 'sig4', interpolation: 'current', timestamp: 'time' }
                ]
            });

            var sig1 = [
                { timestamp: 0, value: 1 },
                { timestamp: 5000, value: 5 },
                { timestamp: 12000, value: 6 },
                { timestamp: 15000, value: 4 }
            ];

            var sig2 = [
                { timestamp: 1000, value: 4 },
                { timestamp: 2000, value: 4 },
                { timestamp: 3000, value: 5 },
                { timestamp: 7000, value: 4 },
                { timestamp: 9000, value: 5 },
                { timestamp: 10000, value: 3 },
                { timestamp: 13000, value: 5 }
            ];

            var sig3 = [
                { timestamp: 0, value: 3 },
                { timestamp: 4000, value: 2 },
                { timestamp: 5000, value: 1 },
                { timestamp: 7000, value: 3 },
                { timestamp: 8000, value: 2 },
                { timestamp: 9000, value: 1 },
                { timestamp: 11000, value: 1 },
                { timestamp: 14000, value: 3 }
            ];

            var sig4 = [
                { timestamp: 2000, value: 6 },
                { timestamp: 6000, value: 0 },
                { timestamp: 11000, value: 3 }
            ];

            var result = [
                 { "$id": 0, "time": "1970-01-01T00:00:02", "sig1": 2.6, "sig2": 4, "sig3": 2.5, "sig4": 6 },
                 { "$id": 1, "time": "1970-01-01T00:00:03", "sig1": 3.4, "sig2": 5, "sig3": 2.25, "sig4": 6 },
                 { "$id": 2, "time": "1970-01-01T00:00:04", "sig1": 4.2, "sig2": 4.75, "sig3": 2, "sig4": 6 },
                 { "$id": 3, "time": "1970-01-01T00:00:05", "sig1": 5, "sig2": 4.5, "sig3": 1, "sig4": 6 },
                 { "$id": 4, "time": "1970-01-01T00:00:06", "sig1": 5.142857, "sig2": 4.25, "sig3": 2, "sig4": 0 },
                 { "$id": 5, "time": "1970-01-01T00:00:07", "sig1": 5.285714, "sig2": 4, "sig3": 3, "sig4": 0 },
                 { "$id": 6, "time": "1970-01-01T00:00:08", "sig1": 5.428571, "sig2": 4.5, "sig3": 2, "sig4": 0 },
                 { "$id": 7, "time": "1970-01-01T00:00:09", "sig1": 5.571429, "sig2": 5, "sig3": 1, "sig4": 0 },
                 { "$id": 8, "time": "1970-01-01T00:00:10", "sig1": 5.714286, "sig2": 3, "sig3": 1, "sig4": 0 },
                 { "$id": 9, "time": "1970-01-01T00:00:11", "sig1": 5.857143, "sig2": 3.666667, "sig3": 1, "sig4": 3 },
                 { "$id": 10, "time": "1970-01-01T00:00:12", "sig1": 6, "sig2": 4.333333, "sig3": 1.666667, "sig4": 3 },
                 { "$id": 11, "time": "1970-01-01T00:00:13", "sig1": 5.333333, "sig2": 5, "sig3": 2.333333, "sig4": 3 }
            ];

            var stores = [base.store('sig1'), base.store('sig2'), base.store('sig3'), base.store('sig4')];
            var vals = [sig1, sig2, sig3, sig4];
            var idxs = [0, 0, 0, 0];

            while (true) {
                var minIdx = findMinVal(idxs, vals);

                if (minIdx < 0) break;

                var store = stores[minIdx];
                var val = vals[minIdx][idxs[minIdx]];
                idxs[minIdx]++;

                var insertObj = {
                    time_ms: val.timestamp,
                    time: new Date(val.timestamp).toISOString().split('Z')[0],
                    value: val.value
                };

                if (verbose) {
                    console.log('inserting: ' + JSON.stringify(insertObj));
                }
                store.push(insertObj);
            }

            var store = base.store('joined');
            var recSet = store.allRecords;

            if (verbose) {
                console.log('Found ' + recSet.length + ' recs...');
            }
            for (var i = 0; i < recSet.length; i++) {
                var rec = recSet[i];
                var resultRec = result[i];

                if (verbose) {
                    console.log(JSON.stringify(rec));
                }
                var test1Res = Math.abs(rec.sig1 - resultRec.sig1) > 1e-4 || Math.abs(rec.sig2 - resultRec.sig2) > 1e-4 || Math.abs(rec.sig3 - resultRec.sig3) > 1e-4 || Math.abs(rec.sig4 - resultRec.sig4) > 1e-4;
                assert.ok(!test1Res, "Merger.Test1");

                //if (Math.abs(rec.sig1 - resultRec.sig1) > 1e-4 || Math.abs(rec.sig2 - resultRec.sig2) > 1e-4 || Math.abs(rec.sig3 - resultRec.sig3) > 1e-4 || Math.abs(rec.sig4 - resultRec.sig4) > 1e-4) {
                //    throw 'Test failed';
                //}
            }

            // test 2
            if (verbose) {
                console.log('TEST 2');
            }
            base.createStore([
                {
                    "name": "sig11",
                    "fields": [
                        { "name": "time_ms", "type": "uint64" },
                        { "name": "time", "type": "datetime" },
                        { "name": "value", "type": "float" }
                    ]
                },
                {
                    "name": "sig12",
                    "fields": [
                        { "name": "time_ms", "type": "uint64" },
                        { "name": "time", "type": "datetime" },
                        { "name": "value", "type": "float" }
                    ]
                },
                {
                    "name": "joined1",
                    "fields": [
                        { "name": "time", "type": "datetime" },
                        { "name": "sig11", "type": "float" },
                        { "name": "sig12", "type": "float" }
                    ]
                }
            ]);

            new qm.StreamAggr(base, {
                type: 'merger',
                name: 'blabla1',
                outStore: 'joined1',
                createStore: false,
                timestamp: 'time',
                fields: [
                    { source: 'sig11', inField: 'value', outField: 'sig11', interpolation: 'current', timestamp: 'time' },
                    { source: 'sig12', inField: 'value', outField: 'sig12', interpolation: 'current', timestamp: 'time' }
                ]
            });

            var sig11 = [
                { timestamp: 0, value: 2 },
                { timestamp: 2000, value: 3 },
                { timestamp: 3000, value: 0 },
                { timestamp: 5000, value: 1 }
            ];

            var sig12 = [
                { timestamp: 1000, value: 1 },
                { timestamp: 3000, value: 2 }
            ];

            var result = [
                { "$id": 0, "time": "1970-01-01T00:00:01", "sig11": 2, "sig12": 1 },
                { "$id": 1, "time": "1970-01-01T00:00:02", "sig11": 3, "sig12": 1 },
                { "$id": 2, "time": "1970-01-01T00:00:03", "sig11": 0, "sig12": 1 },
                { "$id": 3, "time": "1970-01-01T00:00:05", "sig11": 1, "sig12": 2 }
            ]

            var stores = [base.store('sig11'), base.store('sig12')];
            var vals = [sig11, sig12];
            var idxs = [0, 0];

            while (true) {
                var minIdx = findMinVal(idxs, vals);

                if (minIdx < 0) break;

                var store = stores[minIdx];
                var val = vals[minIdx][idxs[minIdx]];
                idxs[minIdx]++;

                var insertObj = {
                    time_ms: val.timestamp,
                    time: new Date(val.timestamp).toISOString().split('Z')[0],
                    value: val.value
                };

                if (verbose) {
                    console.log('inserting: ' + JSON.stringify(insertObj));
                }
                store.push(insertObj);
            }

            var store = base.store('joined1');
            var recSet = store.allRecords;

            if (verbose) {
                console.log('Found ' + recSet.length + ' recs...');
            }
            for (var i = 0; i < recSet.length; i++) {
                var rec = recSet[i];
                var resultRec = result[i];

                if (verbose) {
                    console.log(JSON.stringify(rec));
                }
                var test2Res = Math.abs(rec.sig11 - resultRec.sig11) > 1e-4 || Math.abs(rec.sig12 - resultRec.sig12) > 1e-4;
                assert.ok(!test2Res, "Merger.Test2");

                //if (Math.abs(rec.sig11 - resultRec.sig11) > 1e-4 || Math.abs(rec.sig12 - resultRec.sig12) > 1e-4) {
                //    throw 'Test failed';
                //}
            }

            // test only past
            if (verbose) {
                console.log('TEST 3');
            }
            base.createStore([
                {
                    "name": "sig111",
                    "fields": [
                        { "name": "time_ms", "type": "uint64" },
                        { "name": "time", "type": "datetime" },
                        { "name": "value", "type": "float" }
                    ]
                },
                {
                    "name": "sig112",
                    "fields": [
                        { "name": "time_ms", "type": "uint64" },
                        { "name": "time", "type": "datetime" },
                        { "name": "value", "type": "float" }
                    ]
                },

                {
                    "name": "joined11",
                    "fields": [
                        { "name": "time", "type": "datetime" },
                        { "name": "sig11", "type": "float" },
                        { "name": "sig12", "type": "float" }
                    ]
                }
            ]);

            new qm.StreamAggr(base, {
                type: 'merger',
                name: 'blabla2',
                outStore: 'joined11',
                createStore: false,
                timestamp: 'time',
                onlyPast: true,
                fields: [
                    { source: 'sig111', inField: 'value', outField: 'sig11', interpolation: 'current', timestamp: 'time' },
                    { source: 'sig112', inField: 'value', outField: 'sig12', interpolation: 'current', timestamp: 'time' }
                ]
            });

            var sig111 = [
                { timestamp: 0, value: 2 },
                { timestamp: 2000, value: 3 },
                { timestamp: 3000, value: 0 },
                { timestamp: 5000, value: 1 }
            ];

            var sig112 = [
                { timestamp: 1000, value: 1 },
                { timestamp: 3000, value: 2 }
            ];

            var result = [
                { "$id": 0, "time": "1970-01-01T00:00:01", "sig11": 2, "sig12": 1 },
                { "$id": 1, "time": "1970-01-01T00:00:02", "sig11": 3, "sig12": 1 },
                { "$id": 2, "time": "1970-01-01T00:00:03", "sig11": 0, "sig12": 2 }
            ]

            var stores = [base.store('sig111'), base.store('sig112')];
            var vals = [sig111, sig112];
            var idxs = [0, 0];

            while (true) {
                var minIdx = findMinVal(idxs, vals);

                if (minIdx < 0) break;

                var store = stores[minIdx];
                var val = vals[minIdx][idxs[minIdx]];
                idxs[minIdx]++;

                var insertObj = {
                    time_ms: val.timestamp,
                    time: new Date(val.timestamp).toISOString().split('Z')[0],
                    value: val.value
                };
                if (verbose) {
                    console.log('inserting: ' + JSON.stringify(insertObj));
                }
                store.push(insertObj);
            }

            var store = base.store('joined11');
            var recSet = store.allRecords;

            if (verbose) {
                console.log('Found ' + recSet.length + ' recs...');
            }
            for (var i = 0; i < recSet.length; i++) {
                var rec = recSet[i];
                var resultRec = result[i];

                if (verbose) {
                    console.log(JSON.stringify(rec));
                }
                var test3Res = Math.abs(rec.sig11 - resultRec.sig11) > 1e-4 || Math.abs(rec.sig12 - resultRec.sig12) > 1e-4;
                assert.ok(!test3Res, "Merger.Test3");

                //if (Math.abs(rec.sig11 - resultRec.sig11) > 1e-4 || Math.abs(rec.sig12 - resultRec.sig12) > 1e-4) {
                //    throw 'Test failed';
                //}
            }

            if (verbose) {
                console.log('Success!');
            }
        }

        function testResampler() {
            if (verbose) {
                console.log('Testing resampler...');
            }
            base.createStore([
                {
                    "name": "resampled_in",
                    "fields": [
                        { "name": "time", "type": "datetime" },
                        { "name": "sig1", "type": "float" },
                        { "name": "sig2", "type": "float" },
                        { "name": "sig3", "type": "float" }
                    ]
                },
                {
                    "name": "resampled_out",
                    "fields": [
                        { "name": "time", "type": "datetime" },
                        { "name": "sig1", "type": "float" },
                        { "name": "sig2", "type": "float" },
                        { "name": "sig3", "type": "float" }
                    ]
                }
            ]);

            var rawStore = base.store('resampled_in');
            var resampledStore = base.store('resampled_out');

            rawStore.addStreamAggr({
                name: 'blablahopsasa',
                type: 'resampler',
                outStore: 'resampled_out',
                timestamp: 'time',
                fields: [
                    { name: 'sig1', interpolator: 'previous' },
                    { name: 'sig2', interpolator: 'linear' },
                    { name: 'sig3', interpolator: 'current' }
                ],
                createStore: false,
                interval: 1000
            });

            var times = [
                0,
                2000,
                4000,
                5000,
                5500,
                7000,
                7500,
                9000,
                9100,
                9500,
                10500,
                11000
            ]

            var vals = [
                1,
                3,
                4,
                2,
                3,
                3,
                2,
                0,
                10,
                1,
                3,
                4
            ]

            var result = [
                { "$id": 0, "time": "1970-01-01T00:00:00", "sig1": 1, "sig2": 1, "sig3": 1 },
                { "$id": 1, "time": "1970-01-01T00:00:01", "sig1": 1, "sig2": 2, "sig3": 1 },
                { "$id": 2, "time": "1970-01-01T00:00:02", "sig1": 1, "sig2": 3, "sig3": 3 },
                { "$id": 3, "time": "1970-01-01T00:00:03", "sig1": 3, "sig2": 3.5, "sig3": 3 },
                { "$id": 4, "time": "1970-01-01T00:00:04", "sig1": 3, "sig2": 4, "sig3": 4 },
                { "$id": 5, "time": "1970-01-01T00:00:05", "sig1": 4, "sig2": 2, "sig3": 2 },
                { "$id": 6, "time": "1970-01-01T00:00:06", "sig1": 3, "sig2": 3, "sig3": 3 },
                { "$id": 7, "time": "1970-01-01T00:00:07", "sig1": 3, "sig2": 3, "sig3": 3 },
                { "$id": 8, "time": "1970-01-01T00:00:08", "sig1": 2, "sig2": 1.333333, "sig3": 2 },
                { "$id": 9, "time": "1970-01-01T00:00:09", "sig1": 2, "sig2": 0, "sig3": 0 },
                { "$id": 10, "time": "1970-01-01T00:00:10", "sig1": 1, "sig2": 2, "sig3": 1 },
                { "$id": 11, "time": "1970-01-01T00:00:11", "sig1": 3, "sig2": 4, "sig3": 4 }
            ];

            for (var i = 0; i < vals.length; i++) {
                var insertObj = {
                    time: new Date(times[i]).toISOString().split('Z')[0],
                    sig1: vals[i],
                    sig2: vals[i],
                    sig3: vals[i]
                };

                if (verbose) {
                    console.log('inserting: ' + JSON.stringify(insertObj));
                }
                rawStore.push(insertObj);
            }

            var recSet = resampledStore.allRecords;

            if (verbose) {
                console.log('Found ' + recSet.length + ' recs...');
            }
            for (var i = 0; i < recSet.length; i++) {
                var rec = recSet[i];
                var resultRec = result[i];

                if (verbose) {
                    console.log(JSON.stringify(rec));
                }
                var test4Res = Math.abs(rec.sig1 - resultRec.sig1) > 1e-4 || Math.abs(rec.sig2 - resultRec.sig2) > 1e-4 || Math.abs(rec.sig3 - resultRec.sig3) > 1e-4;
                assert.ok(!test4Res, "Merger.Test4");

                //if (Math.abs(rec.sig1 - resultRec.sig1) > 1e-4 || Math.abs(rec.sig2 - resultRec.sig2) > 1e-4 || Math.abs(rec.sig3 - resultRec.sig3) > 1e-4) {
                //    throw 'Test failed';
                //}
            }

            if (verbose) {
                console.log('Success!');
            }
        }


        testMerger();
        testResampler();

        base.close();

        done();

    })
});

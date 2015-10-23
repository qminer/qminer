/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

var qm = require('qminer');
var assert = require('../../src/nodejs/scripts/assert.js');


describe('Stream Aggregator Tests', function () {
    var base = undefined;
    var store = undefined;

    beforeEach(function () {
        base = new qm.Base({
            mode: 'createClean',
            schema: [
              {
                  name: 'People',
                  fields: [
                      { name: 'Name', type: 'string', primary: true },
                      { name: 'Gendre', type: 'string' }
                  ],
              }
            ]
        });
        store = base.store('People');

    });
    afterEach(function () {
        base.close();
    });

    describe('Constructor Tests', function () {
        it('should construct a new stream aggregator for the base and People store', function () {

            var aggr = new qm.StreamAggr(base, new function () {
                var length = 0;
                this.name = 'nameLength';
                this.onAdd = function (rec) {
                    length = rec.Name.length;
                }
                this.saveJson = function (limit) {
                    return { val: length };
                }
            }, 'People');

            var id1 = base.store('People').push({ Name: "John", Gendre: "Male" });
            assert.equal(aggr.saveJson().val, 4);
        })
        it('should contruct a new stream aggregator for the People store by adding it', function () {
            var s = store.addStreamAggr(new function () {
                var length = 0;
                this.name = 'nameLength';
                this.onAdd = function (rec) {
                    length = rec.Name.length;
                }
                this.saveJson = function (limit) {
                    return { val: length };
                }
            });

            var id1 = base.store('People').push({ Name: "John", Gendre: "Male" });
            assert.equal(s.saveJson().val, 4);

        })
    });

    describe('JsStreamAggr Test', function () {
        it('should register a Js extractor, which counts record.name string length', function () {

            var s = store.addStreamAggr(new function () {
                var data = 0;
                this.name = 'anomaly';
                this.onAdd = function (rec) {
                    //console.log('updated stream aggr for: ', rec.Name);
                    data = rec.Name.length;
                };
                this.saveJson = function (limit) {
                    return { val: data };
                };

                this.getInt = function () {
                    return data;
                };
                this.getFlt = function () {
                    throw 'error';
                };
                this.getTm = function () {

                };

            });


            var id1 = base.store('People').push({ Name: "John", Gendre: "Male" });
            assert(s.saveJson().val == 4);

            var id2 = base.store('People').push({ Name: "Mary", Gendre: "Female" });
            assert(s.saveJson().val == 4);

            var id3 = base.store('People').push({ Name: "Jim", Gendre: "Male" });
            assert(s.saveJson().val == 3);
            assert.throws(
            	function() {
            		s.getFlt() == 3
            });
		    assert.throws(
            	function() {
            		s.getTm() == 3
            });
        })
        it('should register a Js extractor, which counts the number of records in store', function () {

            var s = store.addStreamAggr(new function () {
                var numberOfRecords = 0;
                this.name = 'counter';
                this.onAdd = function (rec) {
                    numberOfRecords++;
                }
                this.onDelete = function (rec) {
                    numberOfRecords--;
                }
                this.saveJson = function (limit) {
                    return { val: numberOfRecords };
                }
            });

            assert.equal(s.saveJson().val, 0);

            var id1 = base.store('People').push({ Name: "John", Gendre: "Male" });
            assert.equal(s.saveJson().val, 1);
            var id2 = base.store('People').push({ Name: "Mary", Gendre: "Female" });
            assert.equal(s.saveJson().val, 2);

            base.store('People').clear();
            assert.equal(s.saveJson().val, 0);
        })
        it('should register a Js extractor, which counts the number of records in store and number of updates', function () {
            var s = store.addStreamAggr(new function () {
                var numberOfRecords = 0;
                var numberOfUpdates = 0;
                this.name = 'counter';
                this.onAdd = function (rec) {
                    numberOfRecords++;
                }
                this.onDelete = function (rec) {
                    numberOfRecords--;
                }
                this.onUpdate = function (rec) {
                    numberOfUpdates++;
                }
                this.saveJson = function (limit) {
                    return { records: numberOfRecords, updates: numberOfUpdates };
                }
            });

            assert.equal(s.saveJson().records, 0);
            assert.equal(s.saveJson().updates, 0);

            var id1 = base.store('People').push({ Name: "John", Gendre: "Male" });
            assert.equal(s.saveJson().records, 1);
            assert.equal(s.saveJson().updates, 0);

            var id2 = base.store('People').push({ Name: "Mary", Gendre: "Female" });
            assert.equal(s.saveJson().records, 2);
            assert.equal(s.saveJson().updates, 0);

            var id2 = base.store('People').push({ Name: "John", Gendre: "Male" });
            assert.equal(s.saveJson().records, 2);
            assert.equal(s.saveJson().updates, 1);
        })
    });
    describe('OnAdd Tests', function () {
        it('should execute the onAdd function and return 4', function () {
            var aggr = new qm.StreamAggr(base, new function () {
                var length = 0;
                this.name = 'nameLength';
                this.onAdd = function (rec) {
                    length = rec.Name.length;
                }
                this.saveJson = function (limit) {
                    return { val: length };
                }
            });

            var id1 = base.store('People').push({ Name: "John", Gendre: "Male" });
            aggr.onAdd(base.store('People')[0]);

            //aggr.onAdd({ Name: "John", Gendre: "Male" }); // doesn't digest a JSON record
            assert.equal(aggr.saveJson().val, 4);
        })        
        it('should throw an exception if the onAdd function is not defined', function () {
            assert.throws(function () {
                var aggr = new qm.StreamAggr(base, new function () {
                    var length = 0;
                    this.name = 'nameLength';
                });
            });
        })
    });
    describe('OnUpdate Tests', function () {
        it('should execute the onUpdate function and return 1', function () {
            var aggr = new qm.StreamAggr(base, new function () {
                var type = null;
                this.name = 'gendreUpdateLength';
                this.onAdd = function (rec) {
                    type = null;
                }
                this.onUpdate = function (rec) {
                    type = rec.Gendre == "Male" ? 0 : 1;
                }
                this.saveJson = function (limit) {
                    return { val: type };
                }
            });

            var id1 = base.store('People').push({ Name: "John", Gendre: "Male" });
            aggr.onAdd(base.store('People')[0]);

            assert.equal(aggr.saveJson().val, null);

            var id2 = base.store('People').push({ Name: "John", Gendre: "Female" });
            aggr.onUpdate(base.store('People')[0]);

            assert.equal(aggr.saveJson().val, 1);
        })
        // unexpectively exits node
        it('should throw an exception if the onAdd function is not defined with the onUpdate', function () {
            assert.throws(function () {
                var aggr = new qm.StreamAggr(base, new function () {
                    var type = null;
                    this.name = 'gendreUpdateLength';
                    this.onUpdate = function (rec) {
                        type = rec.Gendre == "Male" ? 0 : 1;
                    }
                    this.saveJson = function (limit) {
                        return { val: type };
                    }
                });
            });
        })
    });

    describe('SaveJson Tests', function () {
        it('should return a JSON object containing the pair val: length', function () {
            var aggr = new qm.StreamAggr(base, new function () {
                var length = 0;
                this.name = 'nameLength';
                this.onAdd = function (rec) {
                    length = rec.Name.length;
                }
                this.saveJson = function (limit) {
                    return { val: length };
                }
            });
            var id1 = base.store('People').push({ Name: "John", Gendre: "Male" });
            aggr.onAdd(base.store('People')[0]);

            assert.equal(aggr.saveJson().val, 4);
        })
        it('should return a JSON object containing two pairs, for name and for gendre', function () {
            var aggr = new qm.StreamAggr(base, new function () {
                var length = 0;
                var type = null;
                this.name = 'PeopleAggr';
                this.onAdd = function (rec) {
                    length = rec.Name.length;
                    type = rec.Gendre == "Male" ? 0 : 1;
                }
                this.saveJson = function (limit) {
                    return { name: length, gendre: type };
                }
            });
            var id1 = base.store('People').push({ Name: "John", Gendre: "Male" });
            aggr.onAdd(base.store('People')[0]);

            assert.equal(aggr.saveJson().name, 4);
            assert.equal(aggr.saveJson().gendre, 0);
        })
    });

    describe('OnDelete Tests', function () {
        it('should execute the onDelete function and return 1', function () {
            var aggr = new qm.StreamAggr(base, new function () {
                var numberOfDeleted = 0;
                this.name = 'deleteAggr';
                this.onAdd = function (rec) {
                    return;
                }
                this.onDelete = function (rec) {
                    numberOfDeleted++;
                }
                this.saveJson = function (limit) {
                    return { deleted: numberOfDeleted };
                }
            });
            var id1 = base.store('People').push({ Name: "John", Gendre: "Male" });
            aggr.onDelete(base.store('People')[0]);

            assert.equal(aggr.saveJson().deleted, 1);
        })
    });
});

describe('Time Series Window Buffer Tests', function () {
    var base = undefined;
    var store = undefined;
    beforeEach(function () {
        base = new qm.Base({
            mode: 'createClean',
            schema: [{
                name: 'Function',
                fields: [
                    { name: 'Time', type: 'datetime' },
                    { name: 'Value', type: 'float' }
                ]
            }]
        });
        store = base.store('Function');
    });
    afterEach(function () {
        base.close();
    });

    describe('Constructor Tests', function () {
        it('should construct the time series window buffer', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);
            assert.equal(sa.init, false);
        })
        // unexpected node exit
        it('should throw an exception if the keys timestamp and value are missing', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                winsize: 2000
            };
            assert.throws(function () {
                var sa = store.addStreamAggr(aggr);
            });
        })        
    });
    describe('Adding Records Tests', function () {
        it('should update the time and value of the stream aggregate', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            var json = sa.saveJson();
            assert.equal(new Date(json.Time).getTime(), new Date('2015-06-10T14:13:32.0').getTime());
            assert.equal(json.Val, 1);

            store.push({ Time: '2015-06-10T14:17:45.0', Value: 2 });

            json = sa.saveJson();
            assert.equal(new Date(json.Time).getTime(), new Date('2015-06-10T14:17:45.0').getTime());
            assert.equal(json.Val, 2);
        })
    });
    describe('GetFloatVector Tests', function () {
        it('should return the float vector of values in the buffer', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });

            var vec = sa.getFloatVector();
            assert.equal(vec[0], 1);
            assert.equal(vec[1], 2);
        })
        it('should return an empty vector', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);
            var vec = sa.getFloatVector();
            assert.equal(vec.length, 0);
        })
        it('should return the float vector of values that are still in the window', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:34.0', Value: 3 });
            store.push({ Time: '2015-06-10T14:13:35.0', Value: 4 });


            var vec = sa.getFloatVector();
            assert.equal(vec[0], 2);
            assert.equal(vec[1], 3);
            assert.equal(vec[2], 4);
        })
    });
    describe('GetFloatLength, Tests', function () {
        it('should return the length of the float vector containing values in the buffer', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });

            var vec = sa.getFloatVector();
            assert.equal(sa.getFloatLength(), 2);
        })
        it('should return 0 for an empty float vector', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);
            var vec = sa.getFloatVector();
            assert.equal(sa.getFloatLength(), 0);
        })
    });
    describe('GetFloatAt Tests', function () {
        it('should return the value with the index 1', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            assert.equal(sa.getFloatAt(1), 2);
        })
        it('should throw an exception if the vector is empty', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);
            assert.throws(function () {
                var val = sa.getFloat(0);
            });
        })
    })
    describe('GetTimestampVector Tests', function () {
        it('should return a timestamp vector of dates in the buffer', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });

            var vec = sa.getTimestampVector();
            assert.equal(vec.length, 2);
            assert.equal(vec[0] - 11644473600000, new Date('2015-06-10T14:13:32.0').getTime());
            assert.equal(vec[1] - 11644473600000, new Date('2015-06-10T14:13:33.0').getTime());

        })
        it('should return an empty timestamp vector', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);
            var vec = sa.getTimestampVector();
            assert.equal(vec.length, 0);
        })
    });
    describe('GetTimestampLength Tests', function () {
        it('should return the length of the timestamp vector', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            assert.equal(sa.getTimestampLength(), 2);
        })
        it('should return 0 for an empty timestamp vector', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);
            assert.equal(sa.getTimestampLength(), 0);
        })
    });
    describe('GetTimestampAt Tests', function () {
        it('should return the timestamp with index 1', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            assert.equal(sa.getTimestampAt(0) - 11644473600000, new Date('2015-06-10T14:13:32.0').getTime());
            assert.equal(sa.getTimestampAt(1) - 11644473600000, new Date('2015-06-10T14:13:33.0').getTime());
        })
        // throws a C++ exception
        it('should throw an exception for an empty vector', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);
            assert.throws(function () {
                var date = sa.getTimestampAt(0);
            });
        })
    })
    describe('GetInFloat Tests', function () {
        it('should return the newest float in the buffer', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            assert.equal(sa.getInFloat(), 2);
        })
        it('should return 0 for an empty buffer', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);
            assert.equal(sa.getInFloat(), 0);
        })
    });
    describe('GetInTimestamp Tests', function () {
        it('should return the newest timestamp in the buffer', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            assert.equal(sa.getInTimestamp() - 11644473600000, new Date('2015-06-10T14:13:33.0').getTime());
        })
        it('should throw an exception for an empty buffer', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);
            assert.equal(sa.getInTimestamp(), 0);
        })
    });
    describe('GetOutFloatVector Tests', function () {
        it('should return the vector of the leaving values in buffer', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:33.2', Value: 3 });
            store.push({ Time: '2015-06-10T14:13:33.4', Value: 4 });
            store.push({ Time: '2015-06-10T14:13:35.4', Value: 5 });
            var vec = sa.getOutFloatVector();
            assert.equal(vec.length, 3);
            assert.equal(vec[0], 1);
            assert.equal(vec[1], 2);
            assert.equal(vec[2], 3);
        })
        it('should return an empty vector for an empty buffer', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);
            var vec = sa.getOutFloatVector();
            assert.equal(vec.length, 0);
        })
        it('should return an empty vector if all values are still in the window', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:33.2', Value: 3 });
            store.push({ Time: '2015-06-10T14:13:33.4', Value: 4 });
            var vec = sa.getOutFloatVector();
            assert.equal(vec.length, 0);
        })
    });
    describe('GetOutTimestampVector Tests', function () {
        it('should return the vector containing the leaving timestamps of the buffer', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:33.2', Value: 3 });
            store.push({ Time: '2015-06-10T14:13:33.4', Value: 4 });
            store.push({ Time: '2015-06-10T14:13:35.4', Value: 5 });
            var vec = sa.getOutTimestampVector();
            assert.equal(vec.length, 3);
        })
        it('should return an empty vector if the buffer is empty', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);
            var vec = sa.getOutTimestampVector();
            assert.equal(vec.length, 0);
        })
        it('should return an empty vector if all records are all in the window', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:33.2', Value: 3 });
            store.push({ Time: '2015-06-10T14:13:33.4', Value: 4 });
            var vec = sa.getOutTimestampVector();
            assert.equal(vec.length, 0);
        })
    });
    describe('GetNumberOfRecords Tests', function () {
        it('should return the number of records in the buffer', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:33.2', Value: 3 });
            store.push({ Time: '2015-06-10T14:13:33.4', Value: 4 });
            store.push({ Time: '2015-06-10T14:13:35.4', Value: 5 });
            assert.equal(sa.getNumberOfRecords(), 2);
        })
        it('should return 0 if the buffer contains no records', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);;
            assert.equal(sa.getNumberOfRecords(), 0);
        })
    });
    describe('Name Test', function () {
        it('should return the name of the aggregate', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);
            assert.equal(sa.name, 'TimeSeriesWindowAggr');
        })
    });
    describe('Val Test', function () {
        it('should return the json object of the aggregate (same as saveJson)', function () {
            var aggr = {
                name: 'TimeSeriesWindowAggr',
                type: 'timeSeriesWinBuf',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            };
            var sa = store.addStreamAggr(aggr);
            assert.equal(sa.init, false);
        })
    });
});

describe('MovingWindowBufferSum Tests', function () {
    var base = undefined;
    var store = undefined;
    beforeEach(function () {
        base = new qm.Base({
            mode: 'createClean',
            schema: [{
                name: 'Function',
                fields: [
                    { name: 'Time', type: 'datetime' },
                    { name: 'Value', type: 'float' }
                ]
            }]
        });
        store = base.store('Function');

        var aggr = {
            name: 'TimeSeriesWindowAggr',
            type: 'timeSeriesWinBuf',
            store: 'Function',
            timestamp: 'Time',
            value: 'Value',
            winsize: 2000
        }; store.addStreamAggr(aggr);
    });
    afterEach(function () {
        base.close();
    });

    describe('Constructor Tests', function () {
        it('should construct a suma window buffer', function () {
            var aggr = {
                name: 'SumaAggr',
                type: 'winBufSum',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var suma = store.addStreamAggr(aggr);
            assert.equal(suma.init, false);
        })
    });
    describe('GetFloat Tests', function () {
        it('should return the float of the only record in buffer', function () {
            var aggr = {
                name: 'SumaAggr',
                type: 'winBufSum',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var suma = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            assert.equal(suma.getFloat(), 1);
        })
        it('should return 0 if the buffer is empty', function () {
            var aggr = {
                name: 'SumaAggr',
                type: 'winBufSum',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var suma = store.addStreamAggr(aggr);
            assert.equal(suma.getFloat(), 0);
        })
        it('should sum up all values in the window', function () {
            var aggr = {
                name: 'SumaAggr',
                type: 'winBufSum',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var suma = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:33.2', Value: 3 });
            assert.equal(suma.getFloat(), 6);
        })
        it('should sum up all the values except those, that are out of the window', function () {
            var aggr = {
                name: 'SumaAggr',
                type: 'winBufSum',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var suma = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:33.2', Value: 3 });
            store.push({ Time: '2015-06-10T14:13:33.4', Value: 4 });
            store.push({ Time: '2015-06-10T14:13:35.4', Value: 5 });
            assert.equal(suma.getFloat(), 9);
        })
    });
    describe('GetTimestamp Tests', function () {
        it('should return the only timestamp in buffer', function () {
            var aggr = {
                name: 'SumaAggr',
                type: 'winBufSum',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var suma = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            assert.equal(suma.getTimestamp() - 11644473600000, new Date('2015-06-10T14:13:32.0').getTime());
        })
        it('should return the newest timestamp in buffer', function () {
            var aggr = {
                name: 'SumaAggr',
                type: 'winBufSum',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var suma = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:34.0', Value: 3 });
            assert.equal(suma.getTimestamp() - 11644473600000, new Date('2015-06-10T14:13:34.0').getTime());
        })
        it('should reutrn 0 if the buffer is empty', function () {
            var aggr = {
                name: 'SumaAggr',
                type: 'winBufSum',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var suma = store.addStreamAggr(aggr);
            assert.equal(suma.getTimestamp(), 0);
        })
    });
    describe('Property Tests', function () {
        it('should return the name of the suma aggregate', function () {
            var aggr = {
                name: 'SumaAggr',
                type: 'winBufSum',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var suma = store.addStreamAggr(aggr);
            assert.equal(suma.name, 'SumaAggr');
        })
        it('should return the JSON object of the suma aggregate', function () {
            var aggr = {
                name: 'SumaAggr',
                type: 'winBufSum',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var suma = store.addStreamAggr(aggr);
            assert.equal(suma.init, false);
        })
    });
});

describe('MovingWindowBufferMin Tests', function () {
    var base = undefined;
    var store = undefined;
    beforeEach(function () {
        base = new qm.Base({
            mode: 'createClean',
            schema: [{
                name: 'Function',
                fields: [
                    { name: 'Time', type: 'datetime' },
                    { name: 'Value', type: 'float' }
                ]
            }]
        });
        store = base.store('Function');

        var aggr = {
            name: 'TimeSeriesWindowAggr',
            type: 'timeSeriesWinBuf',
            store: 'Function',
            timestamp: 'Time',
            value: 'Value',
            winsize: 2000
        }; store.addStreamAggr(aggr);
    });
    afterEach(function () {
        base.close();
    });

    describe('Constructor Tests', function () {
        // .Val returns a really big number
        it('should construct a min window buffer', function () {
            var aggr = {
                name: 'MinAggr',
                type: 'winBufMin',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var min = store.addStreamAggr(aggr);
            assert.equal(min.init, false);
        })        
        it('should throw an exception if a key value is missing', function () {
            var aggr = {
                name: 'MinAggr',
                type: 'winBufMin',
                store: 'Function',
            };
            assert.throws(function () {
                var min = store.addStreamAggr(aggr);
            });
        })
    });
    describe('GetFloat Tests', function () {
        it('should return the only value in the window', function () {
            var aggr = {
                name: 'MinAggr',
                type: 'winBufMin',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var min = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });

            assert.equal(min.getFloat(), 1);
        })
        it('should return the minimal value in the window', function () {
            var aggr = {
                name: 'MinAggr',
                type: 'winBufMin',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var min = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:34.0', Value: 3 });
            assert.equal(min.getFloat(), 1);
        })
        // getFloat returns a really big number 10+308
        it('should return 0 for an empty window buffer', function () {
            var aggr = {
                name: 'MinAggr',
                type: 'winBufMin',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var min = store.addStreamAggr(aggr);
            assert.notEqual(min.getFloat(), 0);
        })
        it('should return the minimal value in the buffer, that are still in the window', function () {
            var aggr = {
                name: 'MinAggr',
                type: 'winBufMin',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var min = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:33.2', Value: 3 });
            store.push({ Time: '2015-06-10T14:13:33.4', Value: 4 });
            store.push({ Time: '2015-06-10T14:13:35.4', Value: 5 });

            assert.equal(min.getFloat(), 4);
        })
    });

    describe('getTimestamp Tests', function () {
        it('should return the only timestamp on the window', function () {
            var aggr = {
                name: 'MinAggr',
                type: 'winBufMin',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var min = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            assert.equal(min.getTimestamp() - 11644473600000, new Date('2015-06-10T14:13:32.0').getTime());
        })
        it('should return the newest timestamp in the buffer', function () {
            var aggr = {
                name: 'MinAggr',
                type: 'winBufMin',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var min = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:34.0', Value: 3 });
            assert.equal(min.getTimestamp() - 11644473600000, new Date('2015-06-10T14:13:34.0').getTime());
        })
        it('should return the default timestamp if no record is in the buffer', function () {
            var aggr = {
                name: 'MinAggr',
                type: 'winBufMin',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var min = store.addStreamAggr(aggr);
            assert.equal(min.getTimestamp(), 0);
        })
    });

    describe('Properties Tests', function () {
        it('should return the name of the stream aggregator', function () {
            var aggr = {
                name: 'MinAggr',
                type: 'winBufMin',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var min = store.addStreamAggr(aggr);
            assert.equal(min.name, 'MinAggr');
        })
        it('should return the JSON object of the aggregator', function () {
            var aggr = {
                name: 'MinAggr',
                type: 'winBufMin',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var min = store.addStreamAggr(aggr);
            assert.equal(min.init, false);
        })
    });
});

describe('MovingWindowBufferMax Tests', function () {
    var base = undefined;
    var store = undefined;
    beforeEach(function () {
        base = new qm.Base({
            mode: 'createClean',
            schema: [{
                name: 'Function',
                fields: [
                    { name: 'Time', type: 'datetime' },
                    { name: 'Value', type: 'float' }
                ]
            }]
        });
        store = base.store('Function');

        var aggr = {
            name: 'TimeSeriesWindowAggr',
            type: 'timeSeriesWinBuf',
            store: 'Function',
            timestamp: 'Time',
            value: 'Value',
            winsize: 2000
        }; store.addStreamAggr(aggr);
    });
    afterEach(function () {
        base.close();
    });

    describe('Constructor Tests', function () {
        it('should construct a max stream aggregator', function () {
            var aggr = {
                name: 'MaxAggr',
                type: 'winBufMax',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var max = store.addStreamAggr(aggr);
            assert.equal(max.init, false);
        })
        // unexpexted node crash
        it('should throw an exception if some key values are missing', function () {
            var aggr = {
                name: 'MaxAggr',
                type: 'winBufMax',
                store: 'Function',
            };
            assert.throws(function () {
                store.addStreamAggr(aggr);
            })
        })
    });
    describe('GetFloat Tests', function () {
        it('should return the value of the only record in buffer', function () {
            var aggr = {
                name: 'MaxAggr',
                type: 'winBufMax',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var max = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            assert.equal(max.getFloat(), 1);
        })
        it('should return the max value of the records in buffer', function () {
            var aggr = {
                name: 'MaxAggr',
                type: 'winBufMax',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var max = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:34.0', Value: 3 });
            assert.equal(max.getFloat(), 3);
        })
        it('should return the max value of the records, that are still in the window buffer', function () {
            var aggr = {
                name: 'MaxAggr',
                type: 'winBufMax',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var max = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 5 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 4 });
            store.push({ Time: '2015-06-10T14:13:33.2', Value: 3 });
            store.push({ Time: '2015-06-10T14:13:33.4', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:35.4', Value: 1 });

            assert.equal(max.getFloat(), 2);
        })
        it('should return the default value if there are no records in buffer', function () {
            var aggr = {
                name: 'MaxAggr',
                type: 'winBufMax',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var max = store.addStreamAggr(aggr);
            assert.notEqual(max.getFloat(), 0);
        })
    });
    describe('GetTimestamp Tests', function () {
        it('should return the only timestamp on the window', function () {
            var aggr = {
                name: 'MaxAggr',
                type: 'winBufMax',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var max = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            assert.equal(max.getTimestamp() - 11644473600000, new Date('2015-06-10T14:13:32.0').getTime());
        })
        it('should return the newest timestamp in the buffer', function () {
            var aggr = {
                name: 'MaxAggr',
                type: 'winBufMax',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var max = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:34.0', Value: 3 });
            assert.equal(max.getTimestamp() - 11644473600000, new Date('2015-06-10T14:13:34.0').getTime());
        })
        it('should return the default timestamp if no record is in the buffer', function () {
            var aggr = {
                name: 'MaxAggr',
                type: 'winBufMax',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var max = store.addStreamAggr(aggr);
            assert.equal(max.getTimestamp(), 0);
        })
    });

    describe('Properties Tests', function () {
        it('should return the name of the stream aggregator', function () {
            var aggr = {
                name: 'MaxAggr',
                type: 'winBufMax',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var max = store.addStreamAggr(aggr);
            assert.equal(max.name, 'MaxAggr');
        })
        it('should return the JSON object of the aggregator', function () {
            var aggr = {
                name: 'MaxAggr',
                type: 'winBufMax',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var max = store.addStreamAggr(aggr);
            assert.equal(max.init, false);            
        })
    });
});

describe('MovingAverage Tests', function () {
    var base = undefined;
    var store = undefined;
    beforeEach(function () {
        base = new qm.Base({
            mode: 'createClean',
            schema: [{
                name: 'Function',
                fields: [
                    { name: 'Time', type: 'datetime' },
                    { name: 'Value', type: 'float' }
                ]
            }]
        });
        store = base.store('Function');

        var aggr = {
            name: 'TimeSeriesWindowAggr',
            type: 'timeSeriesWinBuf',
            store: 'Function',
            timestamp: 'Time',
            value: 'Value',
            winsize: 2000
        }; store.addStreamAggr(aggr);
    });
    afterEach(function () {
        base.close();
    });
    describe('Constructor Tests', function () {
        it('should construct the moving average aggregator', function () {
            var aggr = {
                name: 'AverageAggr',
                type: 'ma',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var ma = store.addStreamAggr(aggr);
            assert.equal(ma.init, false);
        })        
        it('should throw an exception if some key values are missing', function () {
            var aggr = {
                name: 'AverageAggr',
                type: 'ma',
                store: 'Function',
            };
            assert.throws(function () {
                var ma = store.addStreamAggr(aggr);
            })
        })
    });
    describe('GetFloat Tests', function () {
        it('should get the average of the only record in buffer (which is the same as the value)', function () {
            var aggr = {
                name: 'AverageAggr',
                type: 'ma',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var ma = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            assert.equal(ma.getFloat(), 1);
        })
        it('should get the average of the record values in the buffer', function () {
            var aggr = {
                name: 'AverageAggr',
                type: 'ma',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var ma = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:34.0', Value: 3 });
            assert.equal(ma.getFloat(), 2);
        })
        it('should get the average of the record values, that are still in the window buffer', function () {
            var aggr = {
                name: 'AverageAggr',
                type: 'ma',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var ma = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:33.2', Value: 3 });
            store.push({ Time: '2015-06-10T14:13:33.4', Value: 4 });
            store.push({ Time: '2015-06-10T14:13:35.4', Value: 5 });
            assert.equal(ma.getFloat(), 4.5);
        })
        it('should return 0 if there are no records in the buffer', function () {
            var aggr = {
                name: 'AverageAggr',
                type: 'ma',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var ma = store.addStreamAggr(aggr);
            assert.equal(ma.getFloat(), 0);
        })
    });
    describe('GetTimestamp Tests', function () {
        it('should return the only timestamp on the window', function () {
            var aggr = {
                name: 'AverageAggr',
                type: 'ma',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var ma = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            assert.equal(ma.getTimestamp() - 11644473600000, new Date('2015-06-10T14:13:32.0').getTime());
        })
        it('should return the newest timestamp in the buffer', function () {
            var aggr = {
                name: 'AverageAggr',
                type: 'ma',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var ma = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:34.0', Value: 3 });
            assert.equal(ma.getTimestamp() - 11644473600000, new Date('2015-06-10T14:13:34.0').getTime());
        })
        it('should return the default timestamp if no record is in the buffer', function () {
            var aggr = {
                name: 'AverageAggr',
                type: 'ma',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var ma = store.addStreamAggr(aggr);
            assert.equal(ma.getTimestamp(), 0);
        })
    });
    describe('Property Tests', function () {
        it('should return the name of the aggregator', function () {
            var aggr = {
                name: 'AverageAggr',
                type: 'ma',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var ma = store.addStreamAggr(aggr);
            assert.equal(ma.name, 'AverageAggr');
        })
        it('should return the JSON object of the aggregator', function () {
            var aggr = {
                name: 'AverageAggr',
                type: 'ma',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var ma = store.addStreamAggr(aggr);
            assert.equal(ma.init, false);
        })
    });
});

describe('TimeSeriesTick Tests', function () {
    var base = undefined;
    var store = undefined;
    beforeEach(function () {
        base = new qm.Base({
            mode: 'createClean',
            schema: [{
                name: 'Function',
                fields: [
                    { name: 'Time', type: 'datetime' },
                    { name: 'Value', type: 'float' }
                ]
            }]
        });
        store = base.store('Function');
    });
    afterEach(function () {
        base.close();
    });
    describe('Constructor Tests', function () {
        it('should construct a new aggregator', function () {
            var aggr = {
                name: 'TickAggr',
                type: 'timeSeriesTick',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
            };
            var tick = store.addStreamAggr(aggr);
            assert.equal(tick.init, false);
        })
        it('should throw an exeption if some key values are missing', function () {
            var aggr = {
                name: 'TickAggr',
                type: 'timeSeriesTick',
                store: 'Function',
                value: 'Value',
            };
            assert.throws(function () {
                var tick = store.addStreamAggr(aggr);
            });
        })
    });
    describe('GetFloat Tests', function () {
        it('should return the value of the only record in buffer', function () {
            var aggr = {
                name: 'TickAggr',
                type: 'timeSeriesTick',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
            };
            var tick = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            assert.equal(tick.getFloat(), 1);
        })
        it('should return the value of the newest record in buffer', function () {
            var aggr = {
                name: 'TickAggr',
                type: 'timeSeriesTick',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
            };
            var tick = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:34.0', Value: 3 });
            assert.equal(tick.getFloat(), 3);
        })
        it('should return the value of the newest, still in the window, record of the buffer', function () {
            var aggr = {
                name: 'TickAggr',
                type: 'timeSeriesTick',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
            };
            var tick = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:34.0', Value: 3 });
            store.push({ Time: '2015-06-10T14:13:33.400', Value: 4 });
            store.push({ Time: '2015-06-10T14:13:35.400', Value: 5 });
            assert.equal(tick.getFloat(), 5);
        })
        it('should return 0 if the buffer is empty', function () {
            var aggr = {
                name: 'TickAggr',
                type: 'timeSeriesTick',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
            };
            var tick = store.addStreamAggr(aggr);
            assert.equal(tick.getFloat(), 0);
        })
    });
    describe('GetTimestamp Tests', function () {
        it('should return the default value if the buffer is empty', function () {
            var aggr = {
                name: 'TickAggr',
                type: 'timeSeriesTick',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
            };
            var tick = store.addStreamAggr(aggr);
            assert.equal(tick.getTimestamp(), 0);
        })
        it('should return the timestamp of the only record in the buffer', function () {
            var aggr = {
                name: 'TickAggr',
                type: 'timeSeriesTick',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
            };
            var tick = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            assert.equal(tick.getTimestamp() - 11644473600000, new Date('2015-06-10T14:13:32.0').getTime());
        })
        it('should return the timestamp of the newest record in the buffer', function () {
            var aggr = {
                name: 'TickAggr',
                type: 'timeSeriesTick',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
            };
            var tick = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:34.0', Value: 3 });
            assert.equal(tick.getTimestamp() - 11644473600000, new Date('2015-06-10T14:13:34.0').getTime());
        })
        it('should return the timestamp of the newest, still in the window, record in the buffer', function () {
            var aggr = {
                name: 'TickAggr',
                type: 'timeSeriesTick',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
            };
            var tick = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:34.0', Value: 3 });
            store.push({ Time: '2015-06-10T14:13:33.400', Value: 4 });
            store.push({ Time: '2015-06-10T14:13:35.400', Value: 5 });
            assert.equal(tick.getTimestamp() - 11644473600000, new Date('2015-06-10T14:13:35.400').getTime());
        })
    });
    describe('Property Tests', function () {
        it('should return the name of the aggregator', function () {
            var aggr = {
                name: 'TickAggr',
                type: 'timeSeriesTick',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
            };
            var tick = store.addStreamAggr(aggr);
            assert.equal(tick.name, 'TickAggr');
        })
        it('should return the JSON object of the aggregator', function () {
            var aggr = {
                name: 'TickAggr',
                type: 'timeSeriesTick',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
            };
            var tick = store.addStreamAggr(aggr);
            assert.equal(tick.init, false);
        })
    })
})

describe('EMA Tests', function () {
    var base = undefined;
    var store = undefined;
    beforeEach(function () {
        base = new qm.Base({
            mode: 'createClean',
            schema: [{
                name: 'Function',
                fields: [
                    { name: 'Time', type: 'datetime' },
                    { name: 'Value', type: 'float' }
                ]
            }]
        });
        store = base.store('Function');

        var aggr = {
            name: 'TickAggr',
            type: 'timeSeriesTick',
            store: 'Function',
            timestamp: 'Time',
            value: 'Value',
            winsize: 6000
        }; store.addStreamAggr(aggr);
    });
    afterEach(function () {
        base.close();
    });
    // unexpected node crash
    describe('Constructor Tests', function () {
        it('should construct the ema aggregator', function () {
            var aggr = {
                name: 'EmaAggr',
                type: 'ema',
                store: 'Function',
                inAggr: 'TickAggr',
                emaType: 'previous',
                interval: 3000,
                initWindow: 1000
            };
            var ema = store.addStreamAggr(aggr);
            assert.equal(ema.init, false);
        })
        it('should throw an exception if some key values are missing', function () {
            var aggr = {
                name: 'EmaAggr',
                type: 'ema',
                store: 'Function'
            };
            assert.throws(function () {
                var ema = store.addStreamAggr(aggr);
            });
        })
    });
    describe('GetFloat Tests', function () {
        it('should return 0 if the buffer is empty', function () {
            var aggr = {
                name: 'EmaAggr',
                type: 'ema',
                store: 'Function',
                inAggr: 'TickAggr',
                emaType: 'previous',
                interval: 3000,
                initWindow: 1000
            };
            var ema = store.addStreamAggr(aggr);
            assert.equal(ema.getFloat(), 0);
        })
        it('should return the ema of the only record in buffer with initWindow', function () {
            var aggr = {
                name: 'EmaAggr',
                type: 'ema',
                store: 'Function',
                inAggr: 'TickAggr',
                emaType: 'previous',
                interval: 3000,
                initWindow: 1000
            };
            var ema = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            assert.equal(ema.getFloat(), 0);
        })
        it('should return the ema of the only record in buffer without initWindow', function () {
            var aggr = {
                name: 'EmaAggr',
                type: 'ema',
                store: 'Function',
                inAggr: 'TickAggr',
                emaType: 'previous',
                interval: 3000
            };
            var ema = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            assert.equal(ema.getFloat(), 1);
        })
        it('should return the ema of the records in the buffer with initWindow', function () {
            var aggr = {
                name: 'EmaAggr',
                type: 'ema',
                store: 'Function',
                inAggr: 'TickAggr',
                emaType: 'previous',
                interval: 3000,
                initWindow: 1000
            };
            var ema = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 0 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:34.0', Value: 0 });
            assert.equal(ema.getFloat(), 0);
        })
    });
    describe('GetTimestamp Tests', function () {
        it('should return the default timestamp if the buffer is empty', function () {
            var aggr = {
                name: 'EmaAggr',
                type: 'ema',
                store: 'Function',
                inAggr: 'TickAggr',
                emaType: 'previous',
                interval: 3000,
                initWindow: 1000
            };
            var ema = store.addStreamAggr(aggr);
            assert.equal(ema.getTimestamp(), 0);
        })
        it('should return the timestamp of the only record in buffer', function () {
            var aggr = {
                name: 'EmaAggr',
                type: 'ema',
                store: 'Function',
                inAggr: 'TickAggr',
                emaType: 'previous',
                interval: 3000,
                initWindow: 1000
            };
            var ema = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 0 });
            assert.equal(ema.getTimestamp() - 11644473600000, new Date('2015-06-10T14:13:32.0').getTime());
        })
        it('should return the timestamp of the newest record in the buffer', function () {
            var aggr = {
                name: 'EmaAggr',
                type: 'ema',
                store: 'Function',
                inAggr: 'TickAggr',
                emaType: 'previous',
                interval: 3000,
                initWindow: 1000
            };
            var ema = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:34.0', Value: 3 });
            assert.equal(ema.getTimestamp() - 11644473600000, new Date('2015-06-10T14:13:34.0').getTime());
        })
        it('should return the timestamp of the newest, still in the window, record of the buffer', function () {
            var aggr = {
                name: 'EmaAggr',
                type: 'ema',
                store: 'Function',
                inAggr: 'TickAggr',
                emaType: 'previous',
                interval: 3000,
                initWindow: 1000
            };
            var ema = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:34.0', Value: 3 });
            store.push({ Time: '2015-06-10T14:13:33.400', Value: 4 });
            store.push({ Time: '2015-06-10T14:13:35.400', Value: 5 });
            assert.equal(ema.getTimestamp() - 11644473600000, new Date('2015-06-10T14:13:35.400').getTime());
        })
    });
    describe('Property Tests', function () {
        it('should return the name of the aggregator', function () {
            var aggr = {
                name: 'EmaAggr',
                type: 'ema',
                store: 'Function',
                inAggr: 'TickAggr',
                emaType: 'previous',
                interval: 3000,
                initWindow: 1000
            };
            var ema = store.addStreamAggr(aggr);
            assert.equal(ema.name, 'EmaAggr');
        })
        it('should check that the aggregate is not initialized', function () {
            var aggr = {
                name: 'EmaAggr',
                type: 'ema',
                store: 'Function',
                inAggr: 'TickAggr',
                emaType: 'previous',
                interval: 3000,
                initWindow: 1000
            };
            var ema = store.addStreamAggr(aggr); 
            assert.equal(ema.init, false);                
        })
    });
});

describe('MovingVariance Tests', function () {
    var base = undefined;
    var store = undefined;
    beforeEach(function () {
        base = new qm.Base({
            mode: 'createClean',
            schema: [{
                name: 'Function',
                fields: [
                    { name: 'Time', type: 'datetime' },
                    { name: 'Value', type: 'float' },
                ]
            }]
        });
        store = base.store('Function');

        var aggr = {
            name: 'TimeSeriesWindowAggr',
            type: 'timeSeriesWinBuf',
            store: 'Function',
            timestamp: 'Time',
            value: 'Value',
            winsize: 2000
        }; store.addStreamAggr(aggr);
    });
    afterEach(function () {
        base.close();
    });
    describe('Constructor Tests', function () {
        it('should create a variance aggregator', function () {
            var aggr = {
                name: 'VarAggr',
                type: 'variance',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var variance = store.addStreamAggr(aggr);
            assert.equal(variance.init, false);
        })
        it('should throw an exception if a key value is missing', function () {
            var aggr = {
                name: 'VarAggr',
                type: 'variance',
                store: 'Function',
            };
            assert.throws(function () {
                var variance = store.addStreamAggr(aggr);
            });
        })
    });
    describe('GetFloat Tests', function () {
        it('should get the variance of the single record in buffer', function () {
            var aggr = {
                name: 'VarAggr',
                type: 'variance',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var variance = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            assert.equal(variance.getFloat(), 0);
        })
        it('should get the variance of all the records in the buffer', function () {
            var aggr = {
                name: 'VarAggr',
                type: 'variance',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var variance = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:34.0', Value: 3 });
            assert.equal(variance.getFloat(), 1);
        })
        it('should get the variance of all the records in the buffer, that are still in the window', function () {
            var aggr = {
                name: 'VarAggr',
                type: 'variance',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var variance = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:33.2', Value: 3 });
            store.push({ Time: '2015-06-10T14:13:33.400', Value: 4 });
            store.push({ Time: '2015-06-10T14:13:35.400', Value: 5 });
            assert.equal(variance.getFloat(), 1 / 2);
        })
        it('should return 0 if no records are in the buffer', function () {
            var aggr = {
                name: 'VarAggr',
                type: 'variance',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var variance = store.addStreamAggr(aggr);
            assert.equal(variance.getFloat(), 0);
        })
    });
    describe('GetTimestamp Tests', function () {
        it('should return the timestamp of the only record in buffer', function () {
            var aggr = {
                name: 'VarAggr',
                type: 'variance',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var variance = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            assert.equal(variance.getTimestamp() - 11644473600000, new Date('2015-06-10T14:13:32.0').getTime());
        })
        it('should return the newest timestamp of the buffer', function () {
            var aggr = {
                name: 'VarAggr',
                type: 'variance',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var variance = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:33.200', Value: 3 });
            assert.equal(variance.getTimestamp() - 11644473600000, new Date('2015-06-10T14:13:33.200').getTime()); // look if Date returns the correct value
        })
        it('should return the newest timestamp of the buffer, there are some allready output records', function () {
            var aggr = {
                name: 'VarAggr',
                type: 'variance',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var variance = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:33.200', Value: 3 });
            store.push({ Time: '2015-06-10T14:13:33.400', Value: 4 });
            store.push({ Time: '2015-06-10T14:13:35.400', Value: 5 });
            assert.equal(variance.getTimestamp() - 11644473600000, new Date('2015-06-10T14:13:35.400').getTime()); // look if Date returns the correct value
        })
        it('should return the default timestamp if the buffer is empty', function () {
            var aggr = {
                name: 'VarAggr',
                type: 'variance',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var variance = store.addStreamAggr(aggr);
            assert.equal(variance.getTimestamp(), 0);
        })
    });
    describe('Property Tests', function () {
        it('should return the name of the aggregator', function () {
            var aggr = {
                name: 'VarAggr',
                type: 'variance',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var variance = store.addStreamAggr(aggr);
            assert.equal(variance.name, 'VarAggr');
        })
        it('should return the JSON object of the aggregator', function () {
            var aggr = {
                name: 'VarAggr',
                type: 'variance',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var variance = store.addStreamAggr(aggr);
            assert.equal(variance.init, false);
        })
    });
});

describe('Covariance Tests', function () {
    var base = undefined;
    var store = undefined;
    beforeEach(function () {
        base = new qm.Base({
            mode: 'createClean',
            schema: [{
                name: 'Function',
                fields: [
                    { name: 'Time', type: 'datetime' },
                    { name: 'Value', type: 'float' },
                    { name: 'Value2', type: 'float' }
                ]
            }]
        });
        store = base.store('Function');

        var aggr = {
            name: 'TimeSeries1',
            type: 'timeSeriesWinBuf',
            store: 'Function',
            timestamp: 'Time',
            value: 'Value',
            winsize: 3000
        }; store.addStreamAggr(aggr);

        aggr = {
            name: 'TimeSeries2',
            type: 'timeSeriesWinBuf',
            store: 'Function',
            timestamp: 'Time',
            value: 'Value2',
            winsize: 3000
        }; store.addStreamAggr(aggr);

    });
    afterEach(function () {
        base.close();
    });

    describe('Constructor Tests', function () {
        it('should construct a new aggregator', function () {
            var aggr = {
                name: 'CovAggr',
                type: 'covariance',
                store: 'Function',
                inAggrX: 'TimeSeries1',
                inAggrY: 'TimeSeries2'
            };
            var cov = store.addStreamAggr(aggr);
            assert.equal(cov.init, false);
        })
        it('should throw an exception if some key values are missing', function () {
            var aggr = {
                name: 'CovAggr',
                type: 'covariance',
                store: 'Function',
                inAggrX: 'TimeSeries1',
            };
            assert.throws(function () {
                var cov = store.addStreamAggr(aggr);
            })
        })
    });
    describe('GetFloat Tests', function () {
        it('should return the covariance of the only record in buffer', function () {
            var aggr = {
                name: 'CovAggr',
                type: 'covariance',
                store: 'Function',
                inAggrX: 'TimeSeries1',
                inAggrY: 'TimeSeries2'
            };
            var cov = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1, Value2: 2 });
            assert.equal(cov.getFloat(), 0);
        })
        it('should return the covariance of the records in the buffer', function () {
            var aggr = {
                name: 'CovAggr',
                type: 'covariance',
                store: 'Function',
                inAggrX: 'TimeSeries1',
                inAggrY: 'TimeSeries2'
            };
            var cov = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1, Value2: 2 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2, Value2: -1 });
            store.push({ Time: '2015-06-10T14:13:34.0', Value: 3, Value2: 3 });
            assert.eqtol(cov.getFloat(), 0.458333333333333); // precision gained with a Matlab function
        })
        it('should return the covariance of the records that are still in the window buffer', function () {
            var aggr = {
                name: 'CovAggr',
                type: 'covariance',
                store: 'Function',
                inAggrX: 'TimeSeries1',
                inAggrY: 'TimeSeries2'
            };
            var cov = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1, Value2: 2 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2, Value2: -1 });
            store.push({ Time: '2015-06-10T14:13:34.0', Value: 3, Value2: 3 });
            store.push({ Time: '2015-06-10T14:13:35.0', Value: 4, Value2: 2 });
            store.push({ Time: '2015-06-10T14:13:36.0', Value: 5, Value2: 6 });
            assert.eqtol(cov.getFloat(), 2.72222222222222); // precision gained with a Matlab function
        })
    });
    describe('GetTimestamp Tests', function () {
        it('should return the timestamp of the only record in the buffer', function () {
            var aggr = {
                name: 'CovAggr',
                type: 'covariance',
                store: 'Function',
                inAggrX: 'TimeSeries1',
                inAggrY: 'TimeSeries2'
            };
            var cov = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1, Value2: 2 });
            assert.equal(cov.getTimestamp() - 11644473600000, new Date('2015-06-10T14:13:32.0').getTime());
        })
        it('should return the default timestamp of the buffer', function () {
            var aggr = {
                name: 'CovAggr',
                type: 'covariance',
                store: 'Function',
                inAggrX: 'TimeSeries1',
                inAggrY: 'TimeSeries2'
            };
            var cov = store.addStreamAggr(aggr);
            assert.equal(cov.getTimestamp(), 0);
        })
        it('should return the timestamp of the last record in the buffer', function () {
            var aggr = {
                name: 'CovAggr',
                type: 'covariance',
                store: 'Function',
                inAggrX: 'TimeSeries1',
                inAggrY: 'TimeSeries2'
            };
            var cov = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1, Value2: 2 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2, Value2: -1 });
            store.push({ Time: '2015-06-10T14:13:33.200', Value: 3, Value2: 3 });
            assert.equal(cov.getTimestamp() - 11644473600000, new Date('2015-06-10T14:13:33.200').getTime());
        })
        it('should return the timestamp of the last record in the buffer, that is still in the window', function () {
            var aggr = {
                name: 'CovAggr',
                type: 'covariance',
                store: 'Function',
                inAggrX: 'TimeSeries1',
                inAggrY: 'TimeSeries2'
            };
            var cov = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1, Value2: 2 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2, Value2: -1 });
            store.push({ Time: '2015-06-10T14:13:33.200', Value: 3, Value2: 3 });
            store.push({ Time: '2015-06-10T14:13:33.400', Value: 4, Value2: 2 });
            store.push({ Time: '2015-06-10T14:13:43.400', Value: 5, Value2: 6 });
            assert.equal(cov.getTimestamp() - 11644473600000, new Date('2015-06-10T14:13:43.400').getTime());
        })
    });
    describe('Property Tests', function () {
        it('should return the name of the aggregator', function () {
            var aggr = {
                name: 'CovAggr',
                type: 'covariance',
                store: 'Function',
                inAggrX: 'TimeSeries1',
                inAggrY: 'TimeSeries2'
            };
            var cov = store.addStreamAggr(aggr);
            assert.equal(cov.name, 'CovAggr');
        })
        it('should return the JSON object of the aggregator', function () {
            var aggr = {
                name: 'CovAggr',
                type: 'covariance',
                store: 'Function',
                inAggrX: 'TimeSeries1',
                inAggrY: 'TimeSeries2'
            };
            var cov = store.addStreamAggr(aggr);
            assert.equal(cov.init, false);
        })
    });
});

describe('Correlation Tests', function () {
    var base = undefined;
    var store = undefined;
    beforeEach(function () {
        base = new qm.Base({
            mode: 'createClean',
            schema: [{
                name: 'Function',
                fields: [
                    { name: 'Time', type: 'datetime' },
                    { name: 'Value', type: 'float' },
                    { name: 'Value2', type: 'float' }
                ]
            }]
        });
        store = base.store('Function');

        var aggr = {
            name: 'TimeSeries1',
            type: 'timeSeriesWinBuf',
            store: 'Function',
            timestamp: 'Time',
            value: 'Value',
            winsize: 3000
        }; store.addStreamAggr(aggr);

        aggr = {
            name: 'TimeSeries2',
            type: 'timeSeriesWinBuf',
            store: 'Function',
            timestamp: 'Time',
            value: 'Value2',
            winsize: 3000
        }; store.addStreamAggr(aggr);

        aggr = {
            name: 'CovAggr',
            type: 'covariance',
            store: 'Function',
            inAggrX: 'TimeSeries1',
            inAggrY: 'TimeSeries2'
        }; store.addStreamAggr(aggr);

        aggr = {
            name: 'VarAggrX',
            type: 'variance',
            store: 'Function',
            inAggr: 'TimeSeries1'
        }; store.addStreamAggr(aggr);

        aggr = {
            name: 'VarAggrY',
            type: 'variance',
            store: 'Function',
            inAggr: 'TimeSeries2'
        }; store.addStreamAggr(aggr);

    });
    afterEach(function () {
        base.close();
    });

    describe('Constructor Tests', function () {
        it('should construct a new correlation stream aggregator', function () {
            var aggr = {
                name: 'CorrAggr',
                type: 'correlation',
                store: 'Function',
                inAggrCov: 'CovAggr',
                inAggrVarX: 'VarAggrX',
                inAggrVarY: 'VarAggrY'
            };
            var corr = store.addStreamAggr(aggr);
            assert.equal(corr.init, false);
        })
        // unexpected node crash
        it('should throw an exception if a key-value is missing', function () {
            var aggr = {
                name: 'CorrAggr',
                type: 'correlation',
                store: 'Function',
                inAggrCov: 'CovAggr',
                inAggrVarY: 'VarAggrY'
            };
            assert.throws(function () {
                var corr = store.addStreamAggr(aggr);
            });
        })
    });
    describe('GetFloat Tests', function () {
        it('should return 1 for a correlation buffer with one record', function () {
            var aggr = {
                name: 'CorrAggr',
                type: 'correlation',
                store: 'Function',
                inAggrCov: 'CovAggr',
                inAggrVarX: 'VarAggrX',
                inAggrVarY: 'VarAggrY'
            };
            var corr = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1, Value2: 2 });
            assert.equal(corr.getFloat(), 1);
        })
        it('should return -1 for the two records in the buffer', function () {
            var aggr = {
                name: 'CorrAggr',
                type: 'correlation',
                store: 'Function',
                inAggrCov: 'CovAggr',
                inAggrVarX: 'VarAggrX',
                inAggrVarY: 'VarAggrY'
            };
            var corr = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1, Value2: 2 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2, Value2: -1 });
            assert.eqtol(corr.getFloat(), -0.5);
        })
        it('should return the correlation of the two vectors in the buffer', function () {
            var aggr = {
                name: 'CorrAggr',
                type: 'correlation',
                store: 'Function',
                inAggrCov: 'CovAggr',
                inAggrVarX: 'VarAggrX',
                inAggrVarY: 'VarAggrY'
            };
            var corr = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1, Value2: 2 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2, Value2: -1 });
            store.push({ Time: '2015-06-10T14:13:34.0', Value: 3, Value2: 3 });
            store.push({ Time: '2015-06-10T14:13:35.0', Value: 4, Value2: 2 });
            assert.eqtol(corr.getFloat(), 0.248451997499977);
        })
        it('should return the correlation of the values, that are still in the window', function () {
            var aggr = {
                name: 'CorrAggr',
                type: 'correlation',
                store: 'Function',
                inAggrCov: 'CovAggr',
                inAggrVarX: 'VarAggrX',
                inAggrVarY: 'VarAggrY'
            };
            var corr = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1, Value2: 2 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2, Value2: -1 });
            store.push({ Time: '2015-06-10T14:13:34.0', Value: 3, Value2: 3 });
            store.push({ Time: '2015-06-10T14:13:35.0', Value: 4, Value2: 2 });
            store.push({ Time: '2015-06-10T14:13:36.0', Value: 5, Value2: 6 });
            assert.eqtol(corr.getFloat(), 0.730448872649931);
        })
    });
    describe('GetTimestamp Tests', function () {
        it('should return 0 it there is only one record in the buffer', function () {
            var aggr = {
                name: 'CorrAggr',
                type: 'correlation',
                store: 'Function',
                inAggrCov: 'CovAggr',
                inAggrVarX: 'VarAggrX',
                inAggrVarY: 'VarAggrY'
            };
            var corr = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1, Value2: 2 });
            assert.equal(corr.getTimestamp(), 0);
        })
        it('should return 0 if the buffer is empty', function () {
            var aggr = {
                name: 'CorrAggr',
                type: 'correlation',
                store: 'Function',
                inAggrCov: 'CovAggr',
                inAggrVarX: 'VarAggrX',
                inAggrVarY: 'VarAggrY'
            };
            var corr = store.addStreamAggr(aggr);
            assert.equal(corr.getTimestamp(), 0);
        })
        it('should return 0 if there are more records in the buffer', function () {
            var aggr = {
                name: 'CorrAggr',
                type: 'correlation',
                store: 'Function',
                inAggrCov: 'CovAggr',
                inAggrVarX: 'VarAggrX',
                inAggrVarY: 'VarAggrY'
            };
            var corr = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1, Value2: 2 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2, Value2: -1 });
            store.push({ Time: '2015-06-10T14:13:34.0', Value: 3, Value2: 3 });
            store.push({ Time: '2015-06-10T14:13:35.0', Value: 4, Value2: 2 });
            assert.equal(corr.getTimestamp(), 0);
        })
        it('should return 0 if there are also records outside the window', function () {
            var aggr = {
                name: 'CorrAggr',
                type: 'correlation',
                store: 'Function',
                inAggrCov: 'CovAggr',
                inAggrVarX: 'VarAggrX',
                inAggrVarY: 'VarAggrY'
            };
            var corr = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1, Value2: 2 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2, Value2: -1 });
            store.push({ Time: '2015-06-10T14:13:34.0', Value: 3, Value2: 3 });
            store.push({ Time: '2015-06-10T14:13:35.0', Value: 4, Value2: 2 });
            store.push({ Time: '2015-06-10T14:13:36.0', Value: 5, Value2: 6 });
            assert.equal(corr.getTimestamp(), 0);
        })
    });
    describe('Property Tests', function () {
        it('should return the name of the aggregator', function () {
            var aggr = {
                name: 'CorrAggr',
                type: 'correlation',
                store: 'Function',
                inAggrCov: 'CovAggr',
                inAggrVarX: 'VarAggrX',
                inAggrVarY: 'VarAggrY'
            };
            var corr = store.addStreamAggr(aggr);
            assert.equal(corr.name, 'CorrAggr');
        })
        it('should return the value of the aggregator (same as saveJson())', function () {
            var aggr = {
                name: 'CorrAggr',
                type: 'correlation',
                store: 'Function',
                inAggrCov: 'CovAggr',
                inAggrVarX: 'VarAggrX',
                inAggrVarY: 'VarAggrY'
            };
            var corr = store.addStreamAggr(aggr);
            assert.equal(corr.init, false);
        })
    });
});

describe('Resampler Tests', function () {
    var base = undefined;
    var store = undefined;
    var out = undefined;
    beforeEach(function () {
        base = new qm.Base({
            mode: 'createClean',
            schema: [{
                name: 'Function',
                fields: [
                    { name: 'Time', type: 'datetime' },
                    { name: 'Value', type: 'float' }
                ]
            },
            {
                name: 'outStore',
                fields: [
                    { name: 'Value', type: 'float' },
                    { name: 'Time', type: 'datetime' }
                ]
            }]
        });
        store = base.store('Function');
        out = base.store('outStore');
    });
    afterEach(function () {
        base.close();
    });

    describe('Constructor Tests', function () {
        it('should create a new resampler aggregator', function () {
            var aggr = {
                name: 'ResAggr',
                store: 'Function',
                type: 'resampler',
                outStore: 'outStore',
                timestamp: 'Time',
                fields: [{
                    name: 'Value',
                    interpolator: 'previous'
                }],
                createStore: false,
                interval: 10 * 1000
            };
            var res = store.addStreamAggr(aggr);
            assert.equal(res.name, 'ResAggr');
        })
        // unexpected node crash
        it('should throw an exception if a key-value is missing in the json', function () {
            var aggr = {
                name: 'ResAggr',
                store: 'Function',
                type: 'resampler',
                outStore: 'outStore',
                fields: [{
                    name: 'Value',
                    interpolator: 'previous'
                }],
                createStore: false,
                interval: 10 * 1000
            };
            assert.throws(function () {
                var res = store.addStreamAggr(aggr);
            })
        })
    });
    describe('Interpolator Tests', function () {
        it('should interpolate with the previous value', function () {
            var aggr = {
                name: 'ResAggr',
                store: 'Function',
                type: 'resampler',
                outStore: 'outStore',
                timestamp: 'Time',
                fields: [{
                    name: 'Value',
                    interpolator: 'previous'
                }],
                createStore: false,
                interval: 10 * 1000
            };
            var res = store.addStreamAggr(aggr);
            store.push({ Value: 10, Time: '2015-07-08T14:30:00.0' });
            assert.equal(out[0].Value, 10);
            assert.equal(out[0].Time.getTime(), new Date('2015-07-08T14:30:00.0').getTime());

            store.push({ Value: 15, Time: '2015-07-08T14:30:10.100' });
            assert.equal(out[1].Value, 10);
            assert.equal(out[1].Time.getTime(), new Date('2015-07-08T14:30:10.000').getTime());

            store.push({ Value: 20, Time: '2015-07-08T14:30:20.100' });
            assert.equal(out[2].Value, 15);
            assert.equal(out[2].Time.getTime(), new Date('2015-07-08T14:30:20.000').getTime());
        })
        it('should interpolate with the current value', function () {
            var aggr = {
                name: 'ResAggr',
                store: 'Function',
                type: 'resampler',
                outStore: 'outStore',
                timestamp: 'Time',
                fields: [{
                    name: 'Value',
                    interpolator: 'current'
                }],
                createStore: false,
                interval: 10 * 1000
            };
            var res = store.addStreamAggr(aggr);
            store.push({ Value: 10, Time: '2015-07-08T14:30:00.0' });
            assert.equal(out[0].Value, 10);
            assert.equal(out[0].Time.getTime(), new Date('2015-07-08T14:30:00.0').getTime());

            store.push({ Value: 15, Time: '2015-07-08T14:30:10.1' });
            assert.equal(out[1].Value, 10);
            assert.equal(out[1].Time.getTime(), new Date('2015-07-08T14:30:10.0').getTime());

            store.push({ Value: 20, Time: '2015-07-08T14:30:20.1' });
            assert.equal(out[2].Value, 15);
            assert.equal(out[2].Time.getTime(), new Date('2015-07-08T14:30:20.0').getTime());
        })
        it('should interpolate with the linear interpolation', function () {
            var aggr = {
                name: 'ResAggr',
                store: 'Function',
                type: 'resampler',
                outStore: 'outStore',
                timestamp: 'Time',
                fields: [{
                    name: 'Value',
                    interpolator: 'linear'
                }],
                createStore: false,
                interval: 10 * 1000
            };
            var res = store.addStreamAggr(aggr);
            store.push({ Value: 10, Time: '2015-07-08T14:30:00.0' });
            assert.eqtol(out[0].Value, 10);
            assert.equal(out[0].Time.getTime(), new Date('2015-07-08T14:30:00.0').getTime());

            store.push({ Value: 15, Time: '2015-07-08T14:30:15.0' });
            assert.eqtol(out[1].Value, 40 / 3);
            assert.equal(out[1].Time.getTime(), new Date('2015-07-08T14:30:10.0').getTime());

            store.push({ Value: 20, Time: '2015-07-08T14:30:25.0' });
            assert.eqtol(out[2].Value, 17.5);
            assert.equal(out[2].Time.getTime(), new Date('2015-07-08T14:30:20.0').getTime());
        })
    });
    describe('Property Tests', function () {
        it('should return the name of the resampler aggregator', function () {
            var aggr = {
                name: 'ResAggr',
                store: 'Function',
                type: 'resampler',
                outStore: 'outStore',
                timestamp: 'Time',
                fields: [{
                    name: 'Value',
                    interpolator: 'linear'
                }],
                createStore: false,
                interval: 10 * 1000
            };
            var res = store.addStreamAggr(aggr);
            assert.equal(res.name, 'ResAggr');
        })
    });
});

describe('Merger Tests', function () {
    var base = undefined;
    var strore = undefined;
    beforeEach(function () {
        base = new qm.Base({
            mode: 'createClean',
            schema: [{
                name: 'Cars',
                fields: [
                    { name: 'NumberOfCars', type: 'float' },
                    { name: 'Time', type: 'datetime' }
                ]
            },
            {
                name: 'Temperature',
                fields: [
                    { name: 'Celcius', type: 'float' },
                    { name: 'Time', type: 'datetime' }
                ]
            },
            {
                name: 'Merged',
                fields: [
                    { name: 'NumberOfCars', type: 'float' },
                    { name: 'Celcius', type: 'float' },
                    { name: 'Time', type: 'datetime' }
                ]
            }]
        }); store = base.store('Merged');
    });
    afterEach(function () {
        base.close();
    });

    describe('Constructor Tests', function () {
        it('should create a new merger aggregator', function () {
            var aggr = {
                name: 'MergerAggr',
                type: 'stmerger',
                outStore: 'Merged',
                createStore: false,
                timestamp: 'Time',
                fields: [
                    { source: 'Cars', inField: 'NumberOfCars', outField: 'NumberOfCars', interpolation: 'linear', timestamp: 'Time' },
                    { source: 'Temperature', inField: 'Celcius', outField: 'Celcius', interpolation: 'linear', timestamp: 'Time' }
                ]
            };
            var merger = new qm.StreamAggr(base, aggr);
            assert.equal(merger.name, 'MergerAggr');
        })        
        it('should throw an exception if a key-value is not given', function () {
            var aggr = {
                name: 'MergerAggr',
                type: 'stmerger',
                outStore: 'Merged',
                createStore: false,
                fields: [
                    { source: 'Cars', inField: 'NumberOfCars', outField: 'NumberOfCars', interpolation: 'linear', timestamp: 'Time' },
                    { source: 'Temperature', inField: 'Celcius', outField: 'Celcius', interpolation: 'linear', timestamp: 'Time' }
                ]
            };
            assert.throws(function () {
                var merger = new qm.StreamAggr(base, aggr);
            })
        })
    });
    describe('Merging Tests', function () {
        it('should be an empty "Merged" store at the construction', function () {
            var aggr = {
                name: 'MergerAggr',
                type: 'stmerger',
                outStore: 'Merged',
                createStore: false,
                timestamp: 'Time',
                fields: [
                    { source: 'Cars', inField: 'NumberOfCars', outField: 'NumberOfCars', interpolation: 'linear', timestamp: 'Time' },
                    { source: 'Temperature', inField: 'Celcius', outField: 'Celcius', interpolation: 'linear', timestamp: 'Time' }
                ]
            };
            var merger = new qm.StreamAggr(base, aggr);
            assert.equal(store.length, 0);
        })

        it('should return empty "Merged" store for the first added record in Merged store', function () {
            var aggr = {
                name: 'MergerAggr',
                type: 'stmerger',
                outStore: 'Merged',
                createStore: false,
                timestamp: 'Time',
                fields: [
                    { source: 'Cars', inField: 'NumberOfCars', outField: 'NumberOfCars', interpolation: 'linear', timestamp: 'Time' },
                    { source: 'Temperature', inField: 'Celcius', outField: 'Celcius', interpolation: 'linear', timestamp: 'Time' }
                ]
            };
            var merger = new qm.StreamAggr(base, aggr);
            base.store("Cars").push({ NumberOfCars: 5, Time: '2014-01-01T00:00:00.0' });

            assert.equal(store.length, 0);
        })
        it('should still return an empty "Merged" store if there is only one record in each of the stores', function () {
            var aggr = {
                name: 'MergerAggr',
                type: 'stmerger',
                outStore: 'Merged',
                createStore: false,
                timestamp: 'Time',
                fields: [
                    { source: 'Cars', inField: 'NumberOfCars', outField: 'NumberOfCars', interpolation: 'linear', timestamp: 'Time' },
                    { source: 'Temperature', inField: 'Celcius', outField: 'Celcius', interpolation: 'linear', timestamp: 'Time' }
                ]
            };
            var merger = new qm.StreamAggr(base, aggr);
            base.store("Cars").push({ NumberOfCars: 5, Time: '2014-01-01T00:00:00.0' });
            base.store("Temperature").push({ Celcius: 28.3, Time: '2014-01-01T00:00:01.0' });

            assert.equal(store.length, 0);
        })
        it('should put one record in the "Merged" store', function () {
            var aggr = {
                name: 'MergerAggr',
                type: 'stmerger',
                outStore: 'Merged',
                createStore: false,
                timestamp: 'Time',
                fields: [
                    { source: 'Cars', inField: 'NumberOfCars', outField: 'NumberOfCars', interpolation: 'linear', timestamp: 'Time' },
                    { source: 'Temperature', inField: 'Celcius', outField: 'Celcius', interpolation: 'linear', timestamp: 'Time' }
                ]
            };
            var merger = new qm.StreamAggr(base, aggr);
            base.store("Cars").push({ NumberOfCars: 5, Time: '2014-01-01T00:00:00.0' });
            base.store("Temperature").push({ Celcius: 28.3, Time: '2014-01-01T00:00:01.0' });
            base.store("Cars").push({ NumberOfCars: 15, Time: '2014-01-01T00:00:02.0' });

            assert.equal(store.length, 1);
            assert.equal(store.first.NumberOfCars, 10);
            assert.equal(store.first.Celcius, 28.3);
            //assert.equal(store.first.Time, new Date('2014-01-01T00:00:02.0').getTime());
        })
        it('should put two records in the "Merged" store', function () {
            var aggr = {
                name: 'MergerAggr',
                type: 'stmerger',
                outStore: 'Merged',
                createStore: false,
                timestamp: 'Time',
                fields: [
                    { source: 'Cars', inField: 'NumberOfCars', outField: 'NumberOfCars', interpolation: 'linear', timestamp: 'Time' },
                    { source: 'Temperature', inField: 'Celcius', outField: 'Celcius', interpolation: 'linear', timestamp: 'Time' }
                ]
            };
            var merger = new qm.StreamAggr(base, aggr);
            base.store("Cars").push({ NumberOfCars: 5, Time: '2014-01-01T00:00:00.000' });
            base.store("Temperature").push({ Celcius: 28.3, Time: '2014-01-01T00:00:01.000' });
            base.store("Cars").push({ NumberOfCars: 15, Time: '2014-01-01T00:00:02.000' });
            base.store("Temperature").push({ Celcius: 30.3, Time: '2014-01-01T00:00:03.000' });

            //console.log(store[0].toJSON());

            assert.equal(store.length, 2);
            assert.equal(store[0].NumberOfCars, 10);
            assert.equal(store[0].Celcius, 28.3);
            //assert.equal(store[0].Time.getTime(), 1000);

            assert.equal(store[1].NumberOfCars, 15);
            assert.equal(store[1].Celcius, 29.3);
            //assert.equal(store[1].Time.getTime(), 2000);
        })
        it('should put three records in the "Merged" store', function () {
            var aggr = {
                name: 'MergerAggr',
                type: 'stmerger',
                outStore: 'Merged',
                createStore: false,
                timestamp: 'Time',
                fields: [
                    { source: 'Cars', inField: 'NumberOfCars', outField: 'NumberOfCars', interpolation: 'linear', timestamp: 'Time' },
                    { source: 'Temperature', inField: 'Celcius', outField: 'Celcius', interpolation: 'linear', timestamp: 'Time' }
                ]
            };
            var merger = new qm.StreamAggr(base, aggr);
            base.store("Cars").push({ NumberOfCars: 5, Time: '2014-01-01T00:00:00.000' });
            base.store("Temperature").push({ Celcius: 28.3, Time: '2014-01-01T00:00:01.000' });
            base.store("Temperature").push({ Celcius: 29.7, Time: '2014-01-01T00:00:02.000' });
            base.store("Cars").push({ NumberOfCars: 15, Time: '2014-01-01T00:00:03.000' });
            base.store("Temperature").push({ Celcius: 30.3, Time: '2014-01-01T00:00:04.000' });

            assert.equal(store.length, 3);
            assert.eqtol(store[0].NumberOfCars, 5 + 10 / 3);
            assert.equal(store[0].Celcius, 28.3);
            //assert.equal(store[0].Time.getTime(), 1000);

            assert.eqtol(store[1].NumberOfCars, 5 + 2 * 10 / 3);
            assert.equal(store[1].Celcius, 29.7);
            //assert.equal(store[1].Time.getTime(), 2000);

            assert.equal(store[2].NumberOfCars, 15);
            assert.eqtol(store[2].Celcius, 30);
            //assert.equal(store[1].Time.getTime(), 3000);
        })
        it('should throw an exception if try to merge past events', function () {
            var aggr = {
                name: 'MergerAggr',
                type: 'stmerger',
                outStore: 'Merged',
                createStore: false,
                timestamp: 'Time',
                fields: [
                    { source: 'Cars', inField: 'NumberOfCars', outField: 'NumberOfCars', interpolation: 'linear', timestamp: 'Time' },
                    { source: 'Temperature', inField: 'Celcius', outField: 'Celcius', interpolation: 'linear', timestamp: 'Time' }
                ]
            };
            var merger = new qm.StreamAggr(base, aggr);
            base.store("Cars").push({ NumberOfCars: 5, Time: '2014-01-01T00:00:00.000' });
            base.store("Cars").push({ NumberOfCars: 15, Time: '2014-01-01T00:00:02.000' });
            assert.throws(function () {
                base.store("Temperature").push({ Celcius: 28.3, Time: '2014-01-01T00:00:01.000' });
            });
        })
    });
});

describe('Online Histogram Tests', function () {    
    var base = undefined;
    var store = undefined;
    beforeEach(function () {
        base = new qm.Base({
            mode: 'createClean',
            schema: [{
                name: 'Function',
                fields: [
                    { name: 'Time', type: 'datetime' },
                    { name: 'Value', type: 'float' },
                ]
            }]
        });
        store = base.store('Function');

        var aggrBufJson = {
            name: 'TimeSeriesWindowAggr',
            type: 'timeSeriesWinBuf',
            store: 'Function',
            timestamp: 'Time',
            value: 'Value',
            winsize: 2000
        }; store.addStreamAggr(aggrBufJson);

        var aggrTickJson = {
            name: 'TickAggr',
            type: 'timeSeriesTick',
            store: 'Function',
            timestamp: 'Time',
            value: 'Value',
        }; store.addStreamAggr(aggrTickJson);

    });
    afterEach(function () {
        base.close();
    });
    describe('Updating Tests', function () {
        it('should throw an exception if a key-value is not given', function () {
            var aggrJson = {
                name: 'Histogram',
                type: 'onlineHistogram',
                inAggr: 'TimeSeriesWindowAggr',
                lowerBound: 0,
                upperBound: 10,
                bins: 5,
                addNegInf: false,
                addPosInf: false
            }
            //assert.throws(function () {
            //    var copyJson = JSON.parse(JSON.stringify(aggrJson));
            //    delete copyJson.type;
            //    var aggr = store.addStreamAggr(copyJson);
            //}, /TypeError/);
            assert.throws(function () {
                var copyJson = JSON.parse(JSON.stringify(aggrJson));
                delete copyJson.inAggr;
                var aggr = store.addStreamAggr(copyJson);
            }, /TypeError/);
            assert.throws(function () {
                var copyJson = JSON.parse(JSON.stringify(aggrJson));
                delete copyJson.lowerBound;
                var aggr = store.addStreamAggr(copyJson);
            }, /TypeError/);
            assert.throws(function () {
                var copyJson = JSON.parse(JSON.stringify(aggrJson));
                delete copyJson.upperBound;
                var aggr = store.addStreamAggr(copyJson);
            }, /TypeError/);
            var copyJson = JSON.parse(JSON.stringify(aggrJson));
            
            delete copyJson.name;
            delete copyJson.bins;
            delete copyJson.addNegInf;
            delete copyJson.addPosInf;
            var aggr = store.addStreamAggr(copyJson);
            assert(aggr != undefined);
        });
    });
    describe('Updating Tests', function () {
        it('should create an online buffered histogram', function () {
            var aggrJson = {
                name: 'Histogram',
                type: 'onlineHistogram',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr',
                lowerBound: 0,
                upperBound: 10,
                bins: 5,
                addNegInf: false,
                addPosInf: false
            };
            var aggr = store.addStreamAggr(aggrJson);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: -1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 5 });
            store.push({ Time: '2015-06-10T14:13:33.2', Value: 3 });
            store.push({ Time: '2015-06-10T14:13:33.4', Value: 0 });
            store.push({ Time: '2015-06-10T14:13:35.4', Value: 5 });
            store.push({ Time: '2015-06-10T14:13:38.4', Value: 10 });

            assert.equal(aggr.saveJson().counts[0], 0);
            assert.equal(aggr.saveJson().counts[1], 0);
            assert.equal(aggr.saveJson().counts[2], 0);
            assert.equal(aggr.saveJson().counts[3], 0);
            assert.equal(aggr.saveJson().counts[4], 1);
        });
        it('should create an online histogram', function () {
            var aggrJson = {
                name: 'Histogram',
                type: 'onlineHistogram',
                store: 'Function',
                inAggr: 'TickAggr',
                lowerBound: 0,
                upperBound: 10,
                bins: 5,
                addNegInf: false,
                addPosInf: false
            };
            var aggr = store.addStreamAggr(aggrJson);
            store.push({ Time: '2015-06-10T14:13:32.0', Value: -1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 5 });
            store.push({ Time: '2015-06-10T14:13:33.2', Value: 3 });
            store.push({ Time: '2015-06-10T14:13:33.4', Value: 0 });
            store.push({ Time: '2015-06-10T14:13:35.4', Value: 5 });
            store.push({ Time: '2015-06-10T14:13:38.4', Value: 10 });
            
            assert.equal(aggr.saveJson().counts[0], 1);
            assert.equal(aggr.saveJson().counts[1], 1);
            assert.equal(aggr.saveJson().counts[2], 2);
            assert.equal(aggr.saveJson().counts[3], 0);
            assert.equal(aggr.saveJson().counts[4], 1);

            // check that the interface is OK 
            assert(aggr.getFloatLength() == 5);
            assert(aggr.getFloatAt(2) == 2);
            assert(aggr.getFloatVector()[4] == 1);
        });        
    });
});

describe('ChiSquare Tests', function () {
    var base = undefined;
    var store = undefined;
    var hist1 = undefined;
    var hist2 = undefined;
    beforeEach(function () {
        // create a base with a simple store
        // the store records results of throwing a dice
        // Since changes for each nomber are 1/6, the expacted values have uniform distribution
        // Field Observed records the actual values
        var base = new qm.Base({
            mode: "createClean",
            schema: [
            {
                name: "Dice",
                fields: [
                    { name: "Expected", type: "float" },
                    { name: "Observed", type: "float" },
                    { name: "Time", type: "datetime" }
                ]
            }]
        }); 
        store = base.store('Dice');

        // create a new time series stream aggregator for the 'Dice' store, that takes the expected values of throwing a dice
        // and the timestamp from the 'Time' field. The size of the window is 1 day.
        var timeser = {
            name: 'TimeSeries1',
            type: 'timeSeriesWinBuf',
            store: 'Dice',
            timestamp: 'Time',
            value: 'Expected',
            winsize: 86400000 // one day in miliseconds
        };

        var timeSeries1 = base.store("Dice").addStreamAggr(timeser);

        // create a new time series stream aggregator for the 'Dice' store, that takes the actual values of throwing a dice
        // and the timestamp from the 'Time' field. The size of the window is 1 day.
        timeser = {
            name: 'TimeSeries2',
            type: 'timeSeriesWinBuf',
            store: 'Dice',
            timestamp: 'Time',
            value: 'Observed',
            winsize: 86400000 // one day in miliseconds
        };

        var timeSeries2 = base.store("Dice").addStreamAggr(timeser);

        // add a histogram aggregator, that is connected with the 'TimeSeries1' aggregator
        var aggrJson = {
            name: 'Histogram1',
            type: 'onlineHistogram',
            store: 'Dice',
            inAggr: 'TimeSeries1',
            lowerBound: 1,
            upperBound: 6,
            bins: 6,
            addNegInf: false,
            addPosInf: false
        };

        var hist1 = base.store("Dice").addStreamAggr(aggrJson);

        // add a histogram aggregator, that is connected with the 'TimeSeries2' aggregator
        var aggrJson = {
            name: 'Histogram2',
            type: 'onlineHistogram',
            store: 'Dice',
            inAggr: 'TimeSeries2',
            lowerBound: 1,
            upperBound: 6,
            bins: 6,
            addNegInf: false,
            addPosInf: false
        };

        var hist2 = base.store("Dice").addStreamAggr(aggrJson);
    });
    afterEach(function () {
        base.close();
    }); 
    it('should create an chi square test aggregator', function () {
        
        // add ChiSquare aggregator that connects with Histogram1 with expected values and Histogram2 with actual values
        aggr = {
            name: 'ChiAggr',
            type: 'chiSquare',
            storeX: 'Dice',
            storeY: 'Dice',
            inAggrX: 'Histogram1',
            inAggrY: 'Histogram2',
            degreesOfFreedom: 2
        }
        var chi = store.addStreamAggr(aggr);
        
        // add some values
        // simulating throwing a dice
        store.push({ Time: '2015-06-10T14:13:30.0', Expected: 1, Observed: 1 });
        store.push({ Time: '2015-06-10T14:13:31.0', Expected: 2, Observed: 2 });
        store.push({ Time: '2015-06-10T14:13:32.0', Expected: 3, Observed: 3 });
        store.push({ Time: '2015-06-10T14:13:33.0', Expected: 4, Observed: 4 });
        store.push({ Time: '2015-06-10T14:13:34.0', Expected: 5, Observed: 5 });
        store.push({ Time: '2015-06-10T14:13:35.0', Expected: 6, Observed: 5 });

        store.push({ Time: '2015-06-10T14:13:41.0', Expected: 1, Observed: 5 });
        store.push({ Time: '2015-06-10T14:13:41.0', Expected: 2, Observed: 5 });
        store.push({ Time: '2015-06-10T14:13:42.0', Expected: 3, Observed: 5 });
        store.push({ Time: '2015-06-10T14:13:43.0', Expected: 4, Observed: 5 });
        store.push({ Time: '2015-06-10T14:13:44.0', Expected: 5, Observed: 6 });
        store.push({ Time: '2015-06-10T14:13:45.0', Expected: 6, Observed: 6 });

        store.push({ Time: '2015-06-10T14:13:50.0', Expected: 1, Observed: 6 });
        store.push({ Time: '2015-06-10T14:13:51.0', Expected: 2, Observed: 6 });
        store.push({ Time: '2015-06-10T14:13:52.0', Expected: 3, Observed: 6 });
        store.push({ Time: '2015-06-10T14:13:53.0', Expected: 4, Observed: 6 });
        store.push({ Time: '2015-06-10T14:13:54.0', Expected: 5, Observed: 6 });
        store.push({ Time: '2015-06-10T14:13:55.0', Expected: 6, Observed: 6 });

        // show distribution for expected values
        console.log(hist1);

        // show distribution for observed values
        console.log(hist2);

        // show the P-value. A small P value is evidence that the data are not sampled from the distribution you expected.
        // If P-value is smaller than some expected value (e.g. 0.005), then the null null hypothesis fails.
        console.log("P = " + chi.getFloat());

        // print out the aggregator
        console.log(chi);
    });    
    
});



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
                      { name: 'Gender', type: 'string' }
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
        it('should construct a new stream aggregator for the People store using new qm.StreamAggr', function () {

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

            store.push({ Name: "John", Gender: "Male" });
            assert.equal(aggr.saveJson().val, 4);
        })
        it('should test stream aggr construction using functions', function () {
            var s = store.addStreamAggr(new function () {
                var length = 0;
                this.data = 0;
                this.name = 'nameLength';
                this.onAdd = function (rec) {
                    this.data++;
                    length = rec.Name.length;
                }
                this.saveJson = function (limit) {
                    return { val: length, thisVal: this.data };
                }
            }());

            store.push({ Name: "John", Gender: "Male" });
            assert.equal(s.saveJson().val, 4);
            assert.equal(s.saveJson().thisVal, 1);
        })

        // TODO: how do we enable this without breaking 0.12 builds (the test works but needs ES6)?
        //it('should test stream aggr construction using classes ', function () {
        //    if (Number(process.versions.modules) >= 46) {
        //        class S {
        //            constructor() { this.data = 0; }
        //            onAdd(rec) { this.data++; }
        //            saveJson() { return { thisVal: this.data }; }
        //        }
        //        var s = store.addStreamAggr(new S());
        //        store.push({ Name: "John", Gender: "Male" });
        //        assert.equal(s.saveJson().thisVal, 1);
        //    }
        //});

        it('should test stream aggr construction using objects ', function () {
            var S = {
                onAdd: function (rec) { assert(Object.is(this, S)); this.touched = true; console.log('touched ' + this.touched);},
                saveJson: function () { return { touched: this.touched }; }
            };
            var s = store.addStreamAggr(S);
            store.push({ Name: "John", Gender: "Male" });
            assert(s.saveJson().touched);
        });

        it('should test stream aggr construction using prototypes ', function () {
            var t = null;
            function S() { }
            S.prototype.onAdd = function (rec) { assert(Object.is(this, t)); }
            S.prototype.saveJson = function () { return { } }
            t = new S();
            var s = store.addStreamAggr(t);
            store.push({ Name: "John", Gender: "Male" });
        });
    });

    describe('JsStreamAggr Test', function () {
        it('should test getFloat and getInteger with string input', function () {
            var s = new qm.StreamAggr(base, new function () {
                var data = {};
                this.onAdd = function (rec) {
                    data[rec.Name] = data[rec.Name] == undefined ? 1 : data[rec.Name] + 1;
                };
                this.saveJson = function (limit) {
                    return data;
                };
                this.getFloat = function (name) {
                    return data[name] == undefined ? null : data[name];
                };
                this.getInteger = function (name) {
                    return data[name] == undefined ? null : data[name];
                };
            });
            var rec1 = store.newRecord({ Name: 'John', Gender: 'Male' });
            var rec2 = store.newRecord({ Name: 'Mary', Gender: 'Female' });
            s.onAdd(rec1);
            assert.equal(s.getFloat('John'), 1);
            assert.equal(s.getFloat('Mary'), null);
            assert.equal(s.getInteger('John'), 1);
            assert.equal(s.getInteger('Mary'), null);
            s.onAdd(rec1);
            assert.equal(s.getFloat('John'), 2);
            assert.equal(s.getFloat('Mary'), null);
            assert.equal(s.getInteger('John'), 2);
            assert.equal(s.getInteger('Mary'), null);
            s.onAdd(rec2);
            assert.equal(s.getFloat('John'), 2);
            assert.equal(s.getFloat('Mary'), 1);
            assert.equal(s.getInteger('John'), 2);
            assert.equal(s.getInteger('Mary'), 1);
        });

        it('should test isNameFloat and isNameInteger. getFloat (or getInteger) should always return null when isNameFloat (or isNameInteger) returns false', function () {
            var s = new qm.StreamAggr(base, new function () {
                var data = {};
                this.onAdd = function (rec) {
                    data[rec.Name] = data[rec.Name] == undefined ? 1 : data[rec.Name] + 1;
                };
                this.saveJson = function (limit) {
                    return data;
                };
                this.getFloat = function (name) {
                    return data[name] == undefined ? null : data[name];
                };
                this.getInteger = function (name) {
                    return data[name] == undefined ? null : data[name];
                };
                this.isNameFloat = function (name) {
                    if (name == 'John') {
                        return true;
                    } else {
                        return false;
                    }
                };
                this.isNameInteger = function (name) {
                    if (name == 'John') {
                        return true;
                    } else {
                        return false;
                    }
                };
            });
            var rec1 = store.newRecord({ Name: 'John', Gender: 'Male' });
            var rec2 = store.newRecord({ Name: 'Mary', Gender: 'Female' });
            s.onAdd(rec1);
            assert.equal(s.getFloat('John'), 1);
            assert.equal(s.getFloat('Mary'), null);
            assert.equal(s.getInteger('John'), 1);
            assert.equal(s.getInteger('Mary'), null);
            s.onAdd(rec1);
            assert.equal(s.getFloat('John'), 2);
            assert.equal(s.getFloat('Mary'), null);
            assert.equal(s.getInteger('John'), 2);
            assert.equal(s.getInteger('Mary'), null);
            s.onAdd(rec2);
            assert.equal(s.getFloat('John'), 2);
            assert.equal(s.getFloat('Mary'), null);
            assert.equal(s.getInteger('John'), 2);
            assert.equal(s.getInteger('Mary'), null);
        });

        it('should serialize and deserialize a JS implemented stream aggregate', function () {
            var s = store.addStreamAggr(new function () {
                var data = {};
                this.onAdd = function (rec) {
                    data = { Name: rec.Name, Gender: rec.Gender };
                };
                this.saveJson = function (limit) {
                    return data;
                };
                this.save = function (fout) {
                    fout.writeJson(data);
                };
                this.load = function (fin) {
                    data = fin.readJson();
                };
                this.reset = function () {
                    data = {};
                };
                this.init = function() {
                    return data.Name != undefined;
                }
            });
            assert(!s.init);
            // add a record
            store.push({ Name: 'John', Gender: 'Male' });
            assert(s.init);
            // check state
            var state = s.saveJson();
            assert.equal(state.Name, 'John');
            assert.equal(state.Gender, 'Male');
            // save state
            var fnm = 'js_aggr.bin';
            var fout = qm.fs.openWrite(fnm);
            s.save(fout);
            fout.close();
            // reset state
            s.reset();
            assert.equal(Object.keys(s.saveJson()).length, 0);
            // load state
            var fin = qm.fs.openRead(fnm);
            s.load(fin);
            var restoredState = s.saveJson();
            assert.equal(restoredState.Name, 'John');
            assert.equal(restoredState.Gender, 'Male');
            fin.close();
        });
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


            var id1 = base.store('People').push({ Name: "John", Gender: "Male" });
            assert(s.saveJson().val == 4);

            var id2 = base.store('People').push({ Name: "Mary", Gender: "Female" });
            assert(s.saveJson().val == 4);

            var id3 = base.store('People').push({ Name: "Jim", Gender: "Male" });
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

            var id1 = base.store('People').push({ Name: "John", Gender: "Male" });
            assert.equal(s.saveJson().val, 1);
            var id2 = base.store('People').push({ Name: "Mary", Gender: "Female" });
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

            var id1 = base.store('People').push({ Name: "John", Gender: "Male" });
            assert.equal(s.saveJson().records, 1);
            assert.equal(s.saveJson().updates, 0);

            var id2 = base.store('People').push({ Name: "Mary", Gender: "Female" });
            assert.equal(s.saveJson().records, 2);
            assert.equal(s.saveJson().updates, 0);

            var id2 = base.store('People').push({ Name: "John", Gender: "Male" });
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

            var id1 = base.store('People').push({ Name: "John", Gender: "Male" });
            aggr.onAdd(base.store('People')[0]);

            //aggr.onAdd({ Name: "John", Gender: "Male" }); // doesn't digest a JSON record
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
    describe('Reset Tests', function () {
        it('should reset the stream aggregate to 0', function () {
            var aggr = new qm.StreamAggr(base, new function () {
                var length = 0;
                this.name = 'nameLength';
                this.onAdd = function (rec) {
                    length = rec.Name.length;
                }
                this.saveJson = function (limit) {
                    return { val: length };
                }
                this.reset = function () { length = 0;}
            });

            var id1 = base.store('People').push({ Name: "John", Gender: "Male" });
            aggr.onAdd(base.store('People')[0]);

            aggr.reset();
            assert.equal(aggr.saveJson().val, 0);
        })
    });
    describe('OnUpdate Tests', function () {
        it('should execute the onUpdate function and return 1', function () {
            var aggr = new qm.StreamAggr(base, new function () {
                var type = null;
                this.name = 'GenderUpdateLength';
                this.onAdd = function (rec) {
                    type = null;
                }
                this.onUpdate = function (rec) {
                    type = rec.Gender == "Male" ? 0 : 1;
                }
                this.saveJson = function (limit) {
                    return { val: type };
                }
            });

            var id1 = base.store('People').push({ Name: "John", Gender: "Male" });
            aggr.onAdd(base.store('People')[0]);

            assert.equal(aggr.saveJson().val, null);

            var id2 = base.store('People').push({ Name: "John", Gender: "Female" });
            aggr.onUpdate(base.store('People')[0]);

            assert.equal(aggr.saveJson().val, 1);
        })
        // unexpectively exits node
        it('should throw an exception if the onAdd function is not defined with the onUpdate', function () {
            assert.throws(function () {
                var aggr = new qm.StreamAggr(base, new function () {
                    var type = null;
                    this.name = 'GenderUpdateLength';
                    this.onUpdate = function (rec) {
                        type = rec.Gender == "Male" ? 0 : 1;
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
            var id1 = base.store('People').push({ Name: "John", Gender: "Male" });
            aggr.onAdd(base.store('People')[0]);

            assert.equal(aggr.saveJson().val, 4);
        })
        it('should return a JSON object containing two pairs, for name and for Gender', function () {
            var aggr = new qm.StreamAggr(base, new function () {
                var length = 0;
                var type = null;
                this.name = 'PeopleAggr';
                this.onAdd = function (rec) {
                    length = rec.Name.length;
                    type = rec.Gender == "Male" ? 0 : 1;
                }
                this.saveJson = function (limit) {
                    return { name: length, Gender: type };
                }
            });
            var id1 = base.store('People').push({ Name: "John", Gender: "Male" });
            aggr.onAdd(base.store('People')[0]);

            assert.equal(aggr.saveJson().name, 4);
            assert.equal(aggr.saveJson().Gender, 0);
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
            var id1 = base.store('People').push({ Name: "John", Gender: "Male" });
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
    describe('Updating Only Time Test', function () {
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
            store.push({ Time: '2015-06-10T14:23:32.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:33:32.0', Value: 3 });

            sa.onTime(new Date('2016-02-03T14:13:32.0').getTime());
            assert.equal(new Date(sa.getTimestamp()-0).toString(), new Date('2016-02-03T14:13:32.0').toString());
        })
    });
    describe('Reset Tests', function () {
        it('should reset the stream aggregate', function () {
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

            // RESET
            store.resetStreamAggregates();

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
        it('should throw an exception when calling getFloatVector in a noninitialized state', function () {
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
                var vec = sa.getFloatVector();
            });
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
        it('should throw an exception when calling getFloatLength in a noninitialized state', function () {
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
                var res = sa.getFloatLength();
            });
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
            assert.equal(vec[0] - 0, new Date('2015-06-10T14:13:32.0').getTime());
            assert.equal(vec[1] - 0, new Date('2015-06-10T14:13:33.0').getTime());

        })
        it('should throw an exception when calling getTimestampVector in a noninitialized state', function () {
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
                var res = sa.getTimestampVector();
            });
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
        it('should throw an exception when calling getTimestampLength in a noninitialized state', function () {
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
                var res = sa.getTimestampLength();
            });
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
            assert.equal(sa.getTimestampAt(0) - 0, new Date('2015-06-10T14:13:32.0').getTime());
            assert.equal(sa.getTimestampAt(1) - 0, new Date('2015-06-10T14:13:33.0').getTime());
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
        it('should throw an exception for callin getOutFloatVector on an uninitialized buffer', function () {
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
                var vec = sa.getOutFloatVector();
            });
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
        it('should return throw an exception if getOutTimestampVector on an uninitialized buffer', function () {
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
                var vec = sa.getOutTimestampVector();
            });
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
        it('should throw', function () {
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
                var res = sa.getNumberOfRecords();
            });
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

    it('should handle the case when records skip buffer', function () {
        var aggr = {
            type: 'timeSeriesWinBuf', store: 'Function', timestamp: 'Time', value: 'Value',
            winsize: 1,
            delay: 1,
        };
        var sa = store.addStreamAggr(aggr);
        store.push({ Time: '2015-06-10T00:00:00.000', Value: 0 }); // empty
        store.push({ Time: '2015-06-10T00:00:00.001', Value: 1 }); // 1 value
        store.push({ Time: '2015-06-10T00:00:00.004', Value: 4 }); // never enters the buffer
        store.push({ Time: '2015-06-10T00:00:00.007', Value: 7 }); // empty

        assert.equal(sa.getInFloatVector().length, 0);
        assert.equal(sa.getOutFloatVector().length, 0);
        assert.equal(sa.getFloatVector().length, 0);
    });
    it('should handle the case when records skip buffer variation 2', function () {
        var aggr = {
            type: 'timeSeriesWinBuf', store: 'Function', timestamp: 'Time', value: 'Value',
            winsize: 1,
            delay: 1,
        };
        var sa = store.addStreamAggr(aggr);
        store.push({ Time: '2015-06-10T00:00:00.000', Value: 0 }); // empty
        store.push({ Time: '2015-06-10T00:00:00.001', Value: 1 }); // 1 value
        store.push({ Time: '2015-06-10T00:00:00.003', Value: 3 }); // never enters the buffer, influences the buffer
        store.push({ Time: '2015-06-10T00:00:00.006', Value: 6 }); // empty

        assert.equal(sa.getInFloatVector().length, 0);
        assert.equal(sa.getOutFloatVector().length, 1);
        assert.equal(sa.getFloatVector().length, 0);
    });
    it('should handle the case when records skip buffer variation 3', function () {
        var aggr = {
            type: 'timeSeriesWinBuf', store: 'Function', timestamp: 'Time', value: 'Value',
            winsize: 1,
            delay: 10,
        };
        var sa = store.addStreamAggr(aggr);
        store.push({ Time: '2015-06-10T00:00:00.000', Value: 0 }); // empty
        store.push({ Time: '2015-06-10T00:00:00.010', Value: 1 }); // 1 value
        store.push({ Time: '2015-06-10T00:00:00.015', Value: 2 }); // never enters the buffer
        store.push({ Time: '2015-06-10T00:00:00.016', Value: 3 }); // empty

        assert.equal(sa.getInFloatVector().length, 0);
        assert.equal(sa.getOutFloatVector().length, 0);
        assert.equal(sa.getFloatVector().length, 0);
    });
    it('should handle start empty, stay empty: (Bs | I, O, Be) = (0 | 0, 0, 0): ', function () {
        var aggr = {
            type: 'timeSeriesWinBuf', timestamp: 'Time', value: 'Value',
            winsize: 1,
            delay: 1,
        }; var sa = store.addStreamAggr(aggr);

        store.push({ Time: '2015-06-10T00:00:00.000', Value: 0 }); // empty
        store.push({ Time: '2015-06-10T00:00:00.003', Value: 3 }); // stays empty

        assert.equal(sa.getInFloatVector().length, 0);
        assert.equal(sa.getOutFloatVector().length, 0);
        assert.equal(sa.getFloatVector().length, 0);
    });
    it('should handle start empty, get insert: (Bs | I, O, Be) = (0 | 1, 0, 1): ', function () {
        var aggr = {
            type: 'timeSeriesWinBuf', timestamp: 'Time', value: 'Value',
            winsize: 1,
            delay: 1,
        }; var sa = store.addStreamAggr(aggr);

        store.push({ Time: '2015-06-10T00:00:00.000', Value: 0 }); // empty
        store.push({ Time: '2015-06-10T00:00:00.002', Value: 2 }); // gets insert

        assert.equal(sa.getInFloatVector().length, 1);
        assert.equal(sa.getOutFloatVector().length, 0);
        assert.equal(sa.getFloatVector().length, 1);
    });
    it('should handle start nonempty, no changes: (Bs | I, O, Be) = (1 | 0, 0, 1): ', function () {
        var aggr = {
            type: 'timeSeriesWinBuf', timestamp: 'Time', value: 'Value',
            winsize: 1,
            delay: 2,
        }; var sa = store.addStreamAggr(aggr);

        store.push({ Time: '2015-06-10T00:00:00.000', Value: 0 }); // empty
        store.push({ Time: '2015-06-10T00:00:00.002', Value: 2 }); // now nonempty
        store.push({ Time: '2015-06-10T00:00:00.003', Value: 3 }); // no changes

        assert.equal(sa.getInFloatVector().length, 0);
        assert.equal(sa.getOutFloatVector().length, 0);
        assert.equal(sa.getFloatVector().length, 1);
    });
    it('should handle start nonempty, forget all: (Bs | I, O, Be) = (1 | 0, 1, 0): ', function () {
        var aggr = {
            type: 'timeSeriesWinBuf', timestamp: 'Time', value: 'Value',
            winsize: 1,
            delay: 2,
        }; var sa = store.addStreamAggr(aggr);

        store.push({ Time: '2015-06-10T00:00:00.000', Value: 0 }); // empty
        store.push({ Time: '2015-06-10T00:00:00.002', Value: 2 }); // now nonempty
        store.push({ Time: '2015-06-10T00:00:00.006', Value: 6 }); // forget all

        assert.equal(sa.getInFloatVector().length, 0);
        assert.equal(sa.getOutFloatVector().length, 1);
        assert.equal(sa.getFloatVector().length, 0);
    });
    it('should handle start nonempty, forget some: (Bs | I, O, Be) = (1 | 0, 1, 1): ', function () {
        var aggr = {
            type: 'timeSeriesWinBuf', timestamp: 'Time', value: 'Value',
            winsize: 1,
            delay: 2,
        }; var sa = store.addStreamAggr(aggr);

        store.push({ Time: '2015-06-10T00:00:00.000', Value: 0 }); // empty
        store.push({ Time: '2015-06-10T00:00:00.001', Value: 1 }); // empty
        store.push({ Time: '2015-06-10T00:00:00.003', Value: 3 }); // 2 values
        store.push({ Time: '2015-06-10T00:00:00.004', Value: 4 }); // forget one

        assert.equal(sa.getInFloatVector().length, 0);
        assert.equal(sa.getOutFloatVector().length, 1);
        assert.equal(sa.getFloatVector().length, 1);
    });
    it('should handle start nonempty, insert some: (Bs | I, O, Be) = (1 | 1, 0, 1): ', function () {
        var aggr = {
            type: 'timeSeriesWinBuf', timestamp: 'Time', value: 'Value',
            winsize: 1,
            delay: 1,
        }; var sa = store.addStreamAggr(aggr);

        store.push({ Time: '2015-06-10T00:00:00.000', Value: 0 }); // empty
        store.push({ Time: '2015-06-10T00:00:00.001', Value: 1 }); // 1 val
        store.push({ Time: '2015-06-10T00:00:00.002', Value: 2 }); // 2 values

        assert.equal(sa.getInFloatVector().length, 1);
        assert.equal(sa.getOutFloatVector().length, 0);
        assert.equal(sa.getFloatVector().length, 2);
    });
    it('should handle start nonempty, insert some, forget some: (Bs | I, O, Be) = (1 | 1, 1, 1): ', function () {
        var aggr = {
            type: 'timeSeriesWinBuf', timestamp: 'Time', value: 'Value',
            winsize: 1,
            delay: 1,
        }; var sa = store.addStreamAggr(aggr);

        store.push({ Time: '2015-06-10T00:00:00.000', Value: 0 }); // empty
        store.push({ Time: '2015-06-10T00:00:00.001', Value: 1 }); // 1 val
        store.push({ Time: '2015-06-10T00:00:00.002', Value: 2 }); // 2 values
        store.push({ Time: '2015-06-10T00:00:00.003', Value: 3 }); // 2 values

        assert.equal(sa.getInFloatVector().length, 1);
        assert.equal(sa.getOutFloatVector().length, 1);
        assert.equal(sa.getFloatVector().length, 2);
    });

});

describe('Time Series Window Buffer Vector Tests', function () {
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

    describe('Simple test', function () {
        it('winbufvec.getFloatVector should equal to winbuf.getFloatVector', function () {
            var winbuf = store.addStreamAggr({
                type: 'timeSeriesWinBuf',
                timestamp: 'Time',
                value: 'Value',
                winsize: 2000
            });
            var tick = store.addStreamAggr({
                type: 'timeSeriesTick',
                timestamp: 'Time',
                value: 'Value'
            });
            var winbufvec = store.addStreamAggr({
                type: 'timeSeriesWinBufVector',
                inAggr: tick.name,
                winsize: 2000
            });
            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:33:30.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:33:31.0', Value: 3 });
            store.push({ Time: '2015-06-10T14:33:32.0', Value: 4 });

            assert.equal(winbuf.getFloatLength(), winbufvec.getFloatLength());
            var vec = winbuf.getFloatVector();
            var vec2 = winbufvec.getFloatVector();
            for (var i = 0; i < winbuf.getFloatLength(); i++) {
                assert.equal(vec[i], vec2[i]);
            }
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
    describe('Reset Tests', function () {
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

            store.resetStreamAggregates();

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
            assert.equal(suma.getTimestamp() - 0, new Date('2015-06-10T14:13:32.0').getTime());
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
            assert.equal(suma.getTimestamp() - 0, new Date('2015-06-10T14:13:34.0').getTime());
        })
        it('should return last timestam of newest record', function () {
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
            suma.onTime(new Date('2016-02-03T14:13:34.0').getTime());
            assert.equal(suma.getTimestamp() - 0, new Date('2015-06-10T14:13:34.0').getTime());
        })
        it('should reutrn -11644473600000 if the buffer is empty', function () {
            var aggr = {
                name: 'SumaAggr',
                type: 'winBufSum',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var suma = store.addStreamAggr(aggr);
            assert.equal(suma.getTimestamp(), -11644473600000);
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

    describe('Reset Tests', function () {
        it('should reset the buffer', function () {
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

            store.resetStreamAggregates();

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
            assert.equal(min.getTimestamp() - 0, new Date('2015-06-10T14:13:32.0').getTime());
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
            assert.equal(min.getTimestamp() - 0, new Date('2015-06-10T14:13:34.0').getTime());
        })
        it('should return the default timestamp if no record is in the buffer', function () {
            var aggr = {
                name: 'MinAggr',
                type: 'winBufMin',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var min = store.addStreamAggr(aggr);
            assert.equal(min.getTimestamp(), -11644473600000);
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

    describe('Reset Tests', function () {

        it('should reset the window buffer', function () {
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

            store.resetStreamAggregates();

            store.push({ Time: '2015-06-10T14:13:32.0', Value: 5 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 4 });
            store.push({ Time: '2015-06-10T14:13:33.2', Value: 3 });
            store.push({ Time: '2015-06-10T14:13:33.4', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:35.4', Value: 1 });

            assert.equal(max.getFloat(), 2);
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
            assert.equal(max.getTimestamp() - 0, new Date('2015-06-10T14:13:32.0').getTime());
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
            assert.equal(max.getTimestamp() - 0, new Date('2015-06-10T14:13:34.0').getTime());
        })
        it('should return the default timestamp if no record is in the buffer', function () {
            var aggr = {
                name: 'MaxAggr',
                type: 'winBufMax',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var max = store.addStreamAggr(aggr);
            assert.equal(max.getTimestamp(), -11644473600000);
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

    describe('Reset Tests', function () {
        it('should reset the window buffer', function () {
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

            store.resetStreamAggregates();

            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:33.2', Value: 3 });
            store.push({ Time: '2015-06-10T14:13:33.4', Value: 4 });
            store.push({ Time: '2015-06-10T14:13:35.4', Value: 5 });
            assert.equal(ma.getFloat(), 4.5);
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
            assert.equal(ma.getTimestamp() - 0, new Date('2015-06-10T14:13:32.0').getTime());
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
            assert.equal(ma.getTimestamp() - 0, new Date('2015-06-10T14:13:34.0').getTime());
        })
        it('should return the newest timestamp in the buffer, onTime() does nothing', function () {
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
            ma.onTime(new Date('2016-02-03T14:13:34.0').getTime());
            assert.equal(ma.getTimestamp() - 0, new Date('2015-06-10T14:13:34.0').getTime());
        })
        it('should return the default timestamp if no record is in the buffer', function () {
            var aggr = {
                name: 'AverageAggr',
                type: 'ma',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var ma = store.addStreamAggr(aggr);
            assert.equal(ma.getTimestamp(), -11644473600000);
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
        it('should return the value of the newest record in buffer after onTime()', function () {
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
            tick.onTime(new Date('2015-06-10T14:13:34.0').getTime());
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

    describe('Reset Tests', function () {
        it('should reset the buffer', function () {
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

            store.resetStreamAggregates();

            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:34.0', Value: 3 });
            store.push({ Time: '2015-06-10T14:13:33.400', Value: 4 });
            store.push({ Time: '2015-06-10T14:13:35.400', Value: 5 });
            assert.equal(tick.getFloat(), 5);
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
            assert.equal(tick.getTimestamp(), -11644473600000);
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
            assert.equal(tick.getTimestamp() - 0, new Date('2015-06-10T14:13:32.0').getTime());
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
            assert.equal(tick.getTimestamp() - 0, new Date('2015-06-10T14:13:34.0').getTime());
        })
        it('should return the timestamp given with onTime()', function () {
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
            tick.onTime(new Date('2016-02-03T14:13:34.0').getTime());
            assert.equal(tick.getTimestamp() - 0, new Date('2016-02-03T14:13:34.0').getTime());
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
            assert.equal(tick.getTimestamp() - 0, new Date('2015-06-10T14:13:35.400').getTime());
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
    });
    describe('Manual stream aggr triggers', function () {
        it('should correctly not be updated when we call store.push(rec,false)', function () {
            var aggr = {
                name: 'TickAggr',
                type: 'timeSeriesTick',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
            };
            var tick = store.addStreamAggr(aggr);
            store.push({ Value: 1, Time: "2015-01-01" }, true);
            assert(tick.val.Val == 1);
            store.push({ Value: 2, Time: "2015-01-01" }, false);
            assert(tick.val.Val == 1);
        });
        it('should be updated when we call store.triggerOnAddCallbacks()', function () {
            var aggr = {
                name: 'TickAggr',
                type: 'timeSeriesTick',
                store: 'Function',
                timestamp: 'Time',
                value: 'Value',
            };
            var tick = store.addStreamAggr(aggr);
            store.push({ Value: 2, Time: "2015-01-01" }, false);
            store.triggerOnAddCallbacks();
            assert(tick.val.Val == 2);
            store.push({ Value: 3, Time: "2015-01-01" }, false);
            store.triggerOnAddCallbacks(store.last);
            assert(tick.val.Val == 3);
            store.triggerOnAddCallbacks(store.first.$id);
            assert(tick.val.Val == 2);
        });
    });
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

    describe('GetFloat Tests', function () {
        it('should reset the buffer and return the ema of the only record in buffer without initWindow', function () {
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

            store.resetStreamAggregates();

            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            assert.equal(ema.getFloat(), 1);
        })
        it('complex value test', function () {
            var aggr = {
                name: 'EmaAggr',
                type: 'ema',
                store: 'Function',
                inAggr: 'TickAggr',
                emaType: 'previous',
                interval: 2000,
                initWindow: 0
            };
            var ema = store.addStreamAggr(aggr);
            store.push({ Time: 1000, Value: 1 }); //console.log(ema.getFloat());
            assert.equal(ema.getFloat(), 1);
            store.push({ Time: 2000, Value: 1 }); //console.log(ema.getFloat());
            assert.equal(ema.getFloat(), 1);
            store.push({ Time: 3000, Value: 1 }); //console.log(ema.getFloat());
            assert.equal(ema.getFloat(), 1);
            store.push({ Time: 4000, Value: 2 }); //console.log(ema.getFloat());
            assert.equal(ema.getFloat(), 1);
            store.push({ Time: 5000, Value: 2 }); //console.log(ema.getFloat());
            assert.equal(ema.getFloat().toFixed(6), 1.393469);
            store.push({ Time: 6000, Value: 3 }); //console.log(ema.getFloat());
            assert.equal(ema.getFloat().toFixed(6), 1.632121);
            store.push({ Time: 7000, Value: 3 }); //console.log(ema.getFloat());
            assert.equal(ema.getFloat().toFixed(6), 2.170339);
            store.push({ Time: 8000, Value: 3 }); //console.log(ema.getFloat());
            assert.equal(ema.getFloat().toFixed(6), 2.496785);
            store.push({ Time: 10000, Value: 4 }); //console.log(ema.getFloat());
            assert.equal(ema.getFloat().toFixed(6), 2.814878);
            store.push({ Time: 30000, Value: 5 }); //console.log(ema.getFloat());
            assert.equal(ema.getFloat().toFixed(6), 3.999946);
            store.push({ Time: 31000, Value: 5 }); //console.log(ema.getFloat());
            assert.equal(ema.getFloat().toFixed(6), 4.393437);
        })
        it('complex value with onTime() test', function () {
            var aggr = {
                name: 'EmaAggr',
                type: 'ema',
                store: 'Function',
                inAggr: 'TickAggr',
                emaType: 'previous',
                interval: 2000,
                initWindow: 0
            };
            var ema = store.addStreamAggr(aggr);
            store.push({ Time: 10000, Value: 1 }); //console.log(ema.getFloat());
            store.push({ Time: 20000, Value: 2 }); //console.log(ema.getFloat());
            store.push({ Time: 30000, Value: 3 }); //console.log(ema.getFloat());
            store.push({ Time: 40000, Value: 4 }); //console.log(ema.getFloat());
            store.push({ Time: 50000, Value: 5 }); //console.log(ema.getFloat());
            store.push({ Time: 60000, Value: 6 }); //console.log(ema.getFloat());
            store.push({ Time: 70000, Value: 7 }); //console.log(ema.getFloat());
            store.push({ Time: 80000, Value: 8 }); //console.log(ema.getFloat());
            store.push({ Time: 90000, Value: 9 }); //console.log(ema.getFloat());
            store.push({ Time: 100000, Value: 10 }); //console.log(ema.getFloat());
            for (var i=100001; i<110000; i++) {
                ema.onTime(i);
            }
            assert.equal(ema.getFloat().toFixed(2), 9.99);
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
            assert.equal(ema.getTimestamp(), -11644473600000);
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
            assert.equal(ema.getTimestamp() - 0, new Date('2015-06-10T14:13:32.0').getTime());
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
            assert.equal(ema.getTimestamp() - 0, new Date('2015-06-10T14:13:34.0').getTime());
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
            assert.equal(ema.getTimestamp() - 0, new Date('2015-06-10T14:13:35.400').getTime());
        })
        it('should return the timestamp of the newest, still in the window, record of the buffer despite onTime', function () {
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
            ema.onTime(new Date('2016-02-03T14:13:32.0').getTime());
            assert.equal(ema.getTimestamp() - 0, new Date('2015-06-10T14:13:35.400').getTime());
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

    describe('Reset Tests', function () {

        it('should reset and get the variance of all the records in the buffer', function () {
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

            store.resetStreamAggregates();

            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:34.0', Value: 3 });
            assert.equal(variance.getFloat(), 1);

        })
        it('should reset and get the variance of all the records in the buffer, that are still in the window', function () {
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

            store.resetStreamAggregates();

            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2 });
            store.push({ Time: '2015-06-10T14:13:33.2', Value: 3 });
            store.push({ Time: '2015-06-10T14:13:33.400', Value: 4 });
            store.push({ Time: '2015-06-10T14:13:35.400', Value: 5 });
            assert.equal(variance.getFloat(), 1 / 2);
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
            assert.equal(variance.getTimestamp() - 0, new Date('2015-06-10T14:13:32.0').getTime());
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
            assert.equal(variance.getTimestamp() - 0, new Date('2015-06-10T14:13:33.200').getTime()); // look if Date returns the correct value
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
            assert.equal(variance.getTimestamp() - 0, new Date('2015-06-10T14:13:35.400').getTime()); // look if Date returns the correct value
        })
        it('should return the default timestamp if the buffer is empty', function () {
            var aggr = {
                name: 'VarAggr',
                type: 'variance',
                store: 'Function',
                inAggr: 'TimeSeriesWindowAggr'
            };
            var variance = store.addStreamAggr(aggr);
            assert.equal(variance.getTimestamp(), -11644473600000);
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

    describe('Reset Tests', function () {
        it('should reset and return the covariance of the records in the buffer', function () {
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

            store.resetStreamAggregates();

            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1, Value2: 2 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2, Value2: -1 });
            store.push({ Time: '2015-06-10T14:13:34.0', Value: 3, Value2: 3 });
            assert.eqtol(cov.getFloat(), 0.458333333333333); // precision gained with a Matlab function
        })
        it('should reset and return the covariance of the records that are still in the window buffer', function () {
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

            store.resetStreamAggregates();

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
            assert.equal(cov.getTimestamp() - 0, new Date('2015-06-10T14:13:32.0').getTime());
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
            assert.equal(cov.getTimestamp(), -11644473600000);
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
            assert.equal(cov.getTimestamp() - 0, new Date('2015-06-10T14:13:33.200').getTime());
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
            assert.equal(cov.getTimestamp() - 0, new Date('2015-06-10T14:13:43.400').getTime());
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

    describe('Reset Tests', function () {
        it('should reset and return the correlation of the two vectors in the buffer', function () {
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

            store.resetStreamAggregates();

            store.push({ Time: '2015-06-10T14:13:32.0', Value: 1, Value2: 2 });
            store.push({ Time: '2015-06-10T14:13:33.0', Value: 2, Value2: -1 });
            store.push({ Time: '2015-06-10T14:13:34.0', Value: 3, Value2: 3 });
            store.push({ Time: '2015-06-10T14:13:35.0', Value: 4, Value2: 2 });
            assert.eqtol(corr.getFloat(), 0.248451997499977);
        })
        it('should reset and return the correlation of the values, that are still in the window', function () {
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

            store.resetStreamAggregates();

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
            assert.equal(corr.getTimestamp(), -11644473600000);
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
            assert.equal(corr.getTimestamp(), -11644473600000);
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
            assert.equal(corr.getTimestamp(), -11644473600000);
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
            assert.equal(corr.getTimestamp(), -11644473600000);
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

describe('New resampler tests', function () {
    var base = undefined;
    var store = undefined;

    function assertUpdateSequence(recValArr, updatesArr, store, aggr) {
        var recJsonArr = [];
        for (var i = 0; i < recValArr.length; i++) {
            var recJson = recValArr[i];
            recJsonArr.push(recJson);
        }
        assert.equal(aggr.saveJson().val, 0); // should be 0 at start!
        for (var i = 0; i < recValArr.length; i++) {
            store.push(recJsonArr[i]);
            assert.equal(aggr.saveJson().val, updatesArr[i]);
        }
    }

    beforeEach(function () {
        base = new qm.Base({
            mode: 'createClean',
            schema: [{
                name: 'default',
                fields: [
                    { name: 'timestamp', type: 'datetime' },
                    { name: 'value', type: 'float' }
                ]
            }]
        });
        store = base.store('default');
    });
    afterEach(function () {
        base.close();
    });

    describe('General tests', function () {
        it('should create a new resampler aggregator', function () {
            var raw = new qm.StreamAggr(base, {
                type: 'timeSeriesTick',
                store: store,
                timestamp: 'timestamp',
                value: 'value'
            }, store);

            var resampler = new qm.StreamAggr(base, {
                name: 'resample',
                type: 'resample',
                inAggr: raw,
                interpolator: 'current',
                interval: 1000
            }, store);

            var counter = new qm.StreamAggr(base, (function () {
                var updates = 0;

                var that = {
                    name: 'simple',
                    init: function () { return true; },
                    onAdd: function (rec) { that.onStep(); },
                    onStep: function () { updates++; },
                    saveJson: function (limit) { return { val: updates }; }
                }

                return that;
            })());

            resampler.setParams({ outAggr: counter });

            // do the test
            var updateSeq = [
                { timestamp: 0, value: 1 },
                { timestamp: 10000, value: 1 }
            ];

            assertUpdateSequence(updateSeq, [1, 11], store, counter);
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
                type: 'merger',
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
                type: 'merger',
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
                type: 'merger',
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
                type: 'merger',
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
                type: 'merger',
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
                type: 'merger',
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
                type: 'merger',
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
                type: 'merger',
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
                type: 'merger',
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

describe('Nearest Neighbor Anomaly Detection Tests', function () {
    var base = undefined;
    var store = undefined;
    beforeEach(function () {
        base = new qm.Base({
            mode: 'createClean',
            schema: [{
                name: 'Cars',
                fields: [
                    { name: 'NumberOfCars', type: 'float' },
                    { name: 'Temperature', type: 'float' },
                    { name: 'Precipitation', type: 'float' },
                    { name: 'Time', type: 'datetime' }
                ]
            }]
        });
        store = base.store('Cars');

        var aggr = {
            name: "ftrSpaceAggr",
            type: "featureSpace",
            initCount: 2,
            update: true, full: false, sparse: true,
            featureSpace: [
                { type: "numeric", source: "Cars", field: "NumberOfCars", normalize: "var" },
                { type: "numeric", source: "Cars", field: "Temperature", normalize: "var" },
                { type: "numeric", source: "Cars", field: "Precipitation", normalize: "var" }
            ]
        };
        var ftrSpaceAggr = base.store('Cars').addStreamAggr(aggr);

        var aggr = {
            name: "tickAggr",
            type: "timeSeriesTick",
            store: "Cars",
            timestamp: "Time",
            value: "NumberOfCars"
        };
        tickAggr = base.store('Cars').addStreamAggr(aggr);
    });
    afterEach(function () {
        base.close();
    });

    describe('Constructor Tests', function () {
        it('should create a new NN Anomaly Detection aggregator', function () {
            var aggr = {
                name: 'AnomalyDetectorAggr',
                type: 'nnAnomalyDetector',
                inAggrSpV: 'ftrSpaceAggr',
                inAggrTm: 'tickAggr',
                rate: [0.15, 0.5, 0.7],
                windowSize: 2
            };
            var anomaly = base.store('Cars').addStreamAggr(aggr);
            assert.equal(anomaly.name, 'AnomalyDetectorAggr');
        })
        it('should throw an exception if a key-value is not given', function () {
            var aggr = {
                name: 'AnomalyDetectorAggr',
                type: 'nnAnomalyDetector',
                inAggrSpV: 'ftrSpaceAggr',
                rate: [0.15, 0.5, 0.7],
                windowSize: 2
            };
            assert.throws(function () {
                var merger = new qm.StreamAggr(base, aggr);
            })
        })
    });
    describe('Pass data through the NN Anomaly Detection aggregator', function () {
        it('should get the severity of the alarm', function () {
            var aggr = {
                name: 'AnomalyDetectorAggr',
                type: 'nnAnomalyDetector',
                inAggrSpV: 'ftrSpaceAggr',
                inAggrTm: 'tickAggr',
                rate: [0.15, 0.5, 0.7],
                windowSize: 2
            };
            var anomaly = base.store('Cars').addStreamAggr(aggr);

            store.push({ NumberOfCars: 2, Temperature: 30, Precipitation: 200, Time: "2016-09-07T12:00:00" });
            store.push({ NumberOfCars: 2, Temperature: 30, Precipitation: 200, Time: "2016-09-07T12:01:00" });
            store.push({ NumberOfCars: 2, Temperature: 30, Precipitation: 200, Time: "2016-09-07T12:02:00" });
            store.push({ NumberOfCars: 2, Temperature: 30, Precipitation: 0, Time: "2016-09-07T12:03:00" });

            assert.equal(anomaly.getInteger(), 3);

            store.push({ NumberOfCars: 2, Temperature: 30, Precipitation: 0, Time: "2016-09-07T12:04:00" });
            assert.equal(anomaly.getInteger(), 0);
        })
        it('should get the explanation for the alarm', function () {
            var aggr = {
                name: 'AnomalyDetectorAggr',
                type: 'nnAnomalyDetector',
                inAggrSpV: 'ftrSpaceAggr',
                inAggrTm: 'tickAggr',
                rate: [0.15, 0.5, 0.7],
                windowSize: 2
            };
            var anomaly = base.store('Cars').addStreamAggr(aggr);

            store.push({ NumberOfCars: 2, Temperature: 30, Precipitation: 200, Time: "2016-09-07T12:00:00" });
            store.push({ NumberOfCars: 2, Temperature: 30, Precipitation: 200, Time: "2016-09-07T12:01:00" });
            store.push({ NumberOfCars: 2, Temperature: 30, Precipitation: 200, Time: "2016-09-07T12:02:00" });
            store.push({ NumberOfCars: 2, Temperature: 30, Precipitation: 0, Time: "2016-09-07T12:03:00" });

            assert(anomaly.saveJson().explanation.distance);
            store.push({ NumberOfCars: 2, Temperature: 30, Precipitation: 0, Time: "2016-09-07T12:04:00" });
            assert(!anomaly.saveJson().explanation.distance);
        })
    });
    describe('Save and load tests for NN Anomaly Detection aggregator', function () {
        it('should save and load the anomaly aggregator', function () {
            var aggr = {
                name: 'AnomalyDetectorAggr',
                type: 'nnAnomalyDetector',
                inAggrSpV: 'ftrSpaceAggr',
                inAggrTm: 'tickAggr',
                rate: [0.15, 0.5, 0.7],
                windowSize: 2
            };
            var anomaly = base.store('Cars').addStreamAggr(aggr);

            assert(!anomaly.init);
            assert.equal(anomaly.getInteger(), 0);

            var fs = require('qminer').fs;
            var fout = new fs.FOut("./anomaly.bin");
            anomaly.save(fout);
            fout.close();
            var fin = new fs.FIn("./anomaly.bin");
            anomaly.load(fin);

            assert(!anomaly.init);
            assert.equal(anomaly.getInteger(), 0);
        })
        it('should save and load the anomaly aggregator', function () {
            var aggr = {
                name: 'AnomalyDetectorAggr',
                type: 'nnAnomalyDetector',
                inAggrSpV: 'ftrSpaceAggr',
                inAggrTm: 'tickAggr',
                rate: [0.15, 0.5, 0.7],
                windowSize: 2
            };
            var anomaly = base.store('Cars').addStreamAggr(aggr);

            store.push({ NumberOfCars: 2, Temperature: 30, Precipitation: 200, Time: "2016-09-07T12:00:00" });
            store.push({ NumberOfCars: 2, Temperature: 30, Precipitation: 200, Time: "2016-09-07T12:01:00" });
            store.push({ NumberOfCars: 2, Temperature: 30, Precipitation: 200, Time: "2016-09-07T12:02:00" });
            store.push({ NumberOfCars: 2, Temperature: 30, Precipitation: 0, Time: "2016-09-07T12:03:00" });

            assert.equal(anomaly.getInteger(), 3);

            var fs = require('qminer').fs;
            var fout = new fs.FOut("./anomaly.bin");
            anomaly.save(fout);
            fout.close();
            var fin = new fs.FIn("./anomaly.bin");
            anomaly.load(fin);

            assert.equal(anomaly.getInteger(), 3);
        })
    });
    describe('Reset the NN Anomaly Detection aggregator', function () {
        it('should reset the anomaly aggregator', function () {
            var aggr = {
                name: 'AnomalyDetectorAggr',
                type: 'nnAnomalyDetector',
                inAggrSpV: 'ftrSpaceAggr',
                inAggrTm: 'tickAggr',
                rate: [0.15, 0.5, 0.7],
                windowSize: 2
            };
            var anomaly = base.store('Cars').addStreamAggr(aggr);

            store.push({ NumberOfCars: 2, Temperature: 30, Precipitation: 200, Time: "2016-09-07T12:00:00" });
            store.push({ NumberOfCars: 2, Temperature: 30, Precipitation: 200, Time: "2016-09-07T12:01:00" });
            store.push({ NumberOfCars: 2, Temperature: 30, Precipitation: 200, Time: "2016-09-07T12:02:00" });
            store.push({ NumberOfCars: 2, Temperature: 30, Precipitation: 0, Time: "2016-09-07T12:03:00" });

            assert.equal(anomaly.getInteger(), 3);
            assert.equal(anomaly.saveJson().severity, 3);

            store.resetStreamAggregates();
            assert.equal(anomaly.getInteger(), 0);
            assert.equal(anomaly.saveJson().severity, 0);

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

    describe('Reset Tests', function () {
        it('should reset an online buffered histogram', function () {
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

            store.resetStreamAggregates();

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
        it('should reset an online histogram', function () {
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

            store.resetStreamAggregates();

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

describe('TDigest test', function () {
    var base = undefined;
    var store = undefined;
    var td = undefined;

    beforeEach(function () {
        // create a base with a simple store
        // the store records results of throwing two independent fair dices
        base = new qm.Base({
            mode: "createClean",
            schema: [
            {
                name: "Processor",
                fields: [
                    { name: "Value", type: "float" },
                    { name: "Time", type: "datetime" }
                ]
            }]
        });
        store = base.store('Processor');

        // create a new time series stream aggregator for the 'Dice' store, that takes the expected values of throwing a dice
        // and the timestamp from the 'Time' field. The size of the window is 1 day.
        var tick = {
            name: 'TickAggr',
            type: 'timeSeriesTick',
            store: 'Processor',
            timestamp: 'Time',
            value: 'Value',
            winsize: 1000 // one day in miliseconds
        };
        var timeSeries1 = store.addStreamAggr(tick);
    });
    afterEach(function () {
        base.close();
    });
    describe('Create test', function () {
        it('should create a tdigest test aggregator', function () {
            // add TDigest stream aggregator
            var aggr = {
                name: 'TDigest',
                type: 'tdigest',
                store: 'Processor',
                inAggr: 'TickAggr',
                quantiles: [0.9, 0.95, 0.99, 0.999]
            }
            td = store.addStreamAggr(aggr);

            // add some values
            store.push({ Time: '2015-12-01T14:11:32.0', Value: 0.9948628368 });
            store.push({ Time: '2015-12-01T14:16:32.0', Value: 0.1077458826 });
            store.push({ Time: '2015-12-01T14:14:32.0', Value: 0.9855685823 });
            store.push({ Time: '2015-12-01T14:15:32.0', Value: 0.7796449082 });
            store.push({ Time: '2015-12-01T14:18:32.0', Value: 0.0844943286 });
            store.push({ Time: '2015-12-01T14:19:32.0', Value: 0.187490856 });
            store.push({ Time: '2015-12-01T14:12:32.0', Value: 0.0779815107 });
            store.push({ Time: '2015-12-01T14:17:32.0', Value: 0.8945312691 });
            store.push({ Time: '2015-12-01T14:13:32.0', Value: 0.5574567409 });
            store.push({ Time: '2015-12-01T14:20:32.0', Value: 0.1929709807 });
        });
    });
    describe('Datalib output test', function () {
        it('should test t-digest precision is within 10%', function () {
            // add TDigest stream aggregator
            var aggr = {
                name: 'TDigest',
                type: 'tdigest',
                store: 'Processor',
                inAggr: 'TickAggr',
                quantiles: [0.90, 0.95, 0.99, 0.999],
                minCount: 5
            }

            td = store.addStreamAggr(aggr);
            store.push({ Time: '2015-12-01T14:20:32.0', Value: 0.9948628368 });
            store.push({ Time: '2015-12-01T14:20:33.0', Value: 0.1077458826 });
            store.push({ Time: '2015-12-01T14:20:34.0', Value: 0.9855685823 });
            store.push({ Time: '2015-12-01T14:20:35.0', Value: 0.7796449082 });
            assert(!td.init);
            store.push({ Time: '2015-12-01T14:20:36.0', Value: 0.0844943286 });
            assert(td.init);
            store.push({ Time: '2015-12-01T14:20:37.0', Value: 0.187490856 });
            store.push({ Time: '2015-12-01T14:20:38.0', Value: 0.0779815107 });
            store.push({ Time: '2015-12-01T14:20:39.0', Value: 0.8945312691 });
            store.push({ Time: '2015-12-01T14:20:40.0', Value: 0.5574567409 });

            var result = td.getFloatVector();

            assert(result[0] >= 0.8012572567 && result[0] <= 0.9987427433);
            assert(result[1] >= 0.8568245395 && result[1] <= 1.0);
            assert(result[2] >= 0.8905106194 && result[2] <= 1.0);
            assert(result[3] >= 0.8992346849 && result[3] <= 1.0);
        });
        it('should test t-digest with nodejs datalib output', function () {
            // add TDigest stream aggregator
            var aggr = {
                name: 'TDigest',
                type: 'tdigest',
                store: 'Processor',
                inAggr: 'TickAggr',
                quantiles: [0.90, 0.95, 0.99, 0.999]
            }

            td = store.addStreamAggr(aggr);

            // add values
            store.push({ Time: '2015-12-01T14:20:32.0', Value: 0.9948628368});
            store.push({ Time: '2015-12-01T14:20:33.0', Value: 0.1077458826});
            store.push({ Time: '2015-12-01T14:20:34.0', Value: 0.9855685823});
            store.push({ Time: '2015-12-01T14:20:35.0', Value: 0.7796449082});
            store.push({ Time: '2015-12-01T14:20:36.0', Value: 0.0844943286});
            store.push({ Time: '2015-12-01T14:20:37.0', Value: 0.187490856});
            store.push({ Time: '2015-12-01T14:20:38.0', Value: 0.0779815107});
            store.push({ Time: '2015-12-01T14:20:39.0', Value: 0.8945312691});
            store.push({ Time: '2015-12-01T14:20:40.0', Value: 0.5574567409});
            store.push({ Time: '2015-12-01T14:20:41.0', Value: 0.1929709807});
            store.push({ Time: '2015-12-01T14:20:42.0', Value: 0.9307831991});
            store.push({ Time: '2015-12-01T14:20:43.0', Value: 0.9549126723});
            store.push({ Time: '2015-12-01T14:20:44.0', Value: 0.3991612836});
            store.push({ Time: '2015-12-01T14:20:45.0', Value: 0.295941045});
            store.push({ Time: '2015-12-01T14:20:46.0', Value: 0.3841261603});
            store.push({ Time: '2015-12-01T14:20:47.0', Value: 0.6689624672});
            store.push({ Time: '2015-12-01T14:20:48.0', Value: 0.477037447});
            store.push({ Time: '2015-12-01T14:20:49.0', Value: 0.3977089832});
            store.push({ Time: '2015-12-01T14:20:50.0', Value: 0.2465190131});
            store.push({ Time: '2015-12-01T14:20:51.0', Value: 0.7456648378});
            store.push({ Time: '2015-12-01T14:20:52.0', Value: 0.9979597451});
            store.push({ Time: '2015-12-01T14:20:53.0', Value: 0.5717752152});
            store.push({ Time: '2015-12-01T14:20:54.0', Value: 0.7872615189});
            store.push({ Time: '2015-12-01T14:20:55.0', Value: 0.9502113182});

            var result = td.getFloatVector();

            assert.equal(result[0], 0.9822256766499999);
            assert.equal(result[1], 0.99517217467);
            assert.equal(result[2], 0.997588116104);
            assert.equal(result[3], 0.9979225822004);
        });
    });
    describe('Input stress test', function () {
        it('should test t-digest for 10000 inserts', function () {
            // add TDigest stream aggregator
            var aggr = {
                name: 'TDigest',
                type: 'tdigest',
                store: 'Processor',
                inAggr: 'TickAggr',
                quantiles: [0.90, 0.95, 0.99, 0.999]
            }

            td = store.addStreamAggr(aggr);

            function getRnd(min, max) {
                return Math.random() * (max - min) + min;
            }

            for (var i=1; i<=10000; i++) {
                store.push({ Time: '2015-12-01T14:20:32.0', Value: getRnd(0,1) });
            }

            var result = td.getFloatVector();

            assert(result[0] > 0.8 && result[0] < 1);
            assert(result[1] > 0.8 && result[1] < 1);
            assert(result[2] > 0.8 && result[2] < 1);
            assert(result[3] > 0.8 && result[3] < 1);
        });
        it('should test t-digest for 10000 sequential inserts', function () {
            // add TDigest stream aggregator
            var aggr = {
                name: 'TDigest',
                type: 'tdigest',
                store: 'Processor',
                inAggr: 'TickAggr',
                quantiles: [0.90, 0.95, 0.99, 0.999]
            }

            td = store.addStreamAggr(aggr);

            for (var i=1; i<=10000; i++) {
                store.push({ Time: '2015-12-01T14:20:32.0', Value: i });
            }

            var result = td.getFloatVector();
            assert(result[0] > 9000.0 && result[0] < 10000.0);
            assert(result[1] > 9000.0 && result[1] < 10000.0);
            assert(result[2] > 9000.0 && result[2] < 10000.0);
            assert(result[3] > 9900.0 && result[3] < 10000.0);
        });
    });
    describe('Save and load test', function () {
        it('should test t-digest save and load', function () {
            // add TDigest stream aggregator
            var aggr = {
                name: 'TDigest',
                type: 'tdigest',
                store: 'Processor',
                inAggr: 'TickAggr',
                quantiles: [0.90, 0.95, 0.99, 0.999]
            }

            td = store.addStreamAggr(aggr);

            store.push({ Time: '2015-12-01T14:20:32.0', Value: 0.9948628368 });
            store.push({ Time: '2015-12-01T14:20:33.0', Value: 0.1077458826 });
            store.push({ Time: '2015-12-01T14:20:34.0', Value: 0.9855685823 });
            store.push({ Time: '2015-12-01T14:20:35.0', Value: 0.7796449082 });
            store.push({ Time: '2015-12-01T14:20:36.0', Value: 0.0844943286 });
            store.push({ Time: '2015-12-01T14:20:37.0', Value: 0.187490856  });
            store.push({ Time: '2015-12-01T14:20:38.0', Value: 0.0779815107 });
            store.push({ Time: '2015-12-01T14:20:39.0', Value: 0.8945312691 });
            store.push({ Time: '2015-12-01T14:20:40.0', Value: 0.5574567409 });

            var result = td.getFloatVector();

            var fout = qm.fs.openWrite("aggr.tmp");
            td.save(fout);
            fout.close();

            var aggrNew = {
                name: 'TDigestNew',
                type: 'tdigest',
                store: 'Processor',
                inAggr: 'TickAggr',
                quantiles: [0.90, 0.95, 0.99, 0.999]
            }

            var td1 = store.addStreamAggr(aggrNew);

            var fin = qm.fs.openRead("aggr.tmp");
            td1.load(fin);
            fin.close();

            var result1 = td1.getFloatVector();

            assert.equal(result[0], result1[0]);
            assert.equal(result[1], result1[1]);
            assert.equal(result[2], result1[2]);
            assert.equal(result[3], result1[3]);
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
        // the store records results of throwing two independent fair dices
        base = new qm.Base({
            mode: "createClean",
            schema: [
            {
                name: "Dice",
                fields: [
                    { name: "Sample1", type: "float" },
                    { name: "Sample2", type: "float" },
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
            value: 'Sample1',
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
            value: 'Sample2',
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
            upperBound: 7,
            bins: 6,
            addNegInf: false,
            addPosInf: false
        };

        hist1 = base.store("Dice").addStreamAggr(aggrJson);

        // add a histogram aggregator, that is connected with the 'TimeSeries2' aggregator
        var aggrJson = {
            name: 'Histogram2',
            type: 'onlineHistogram',
            store: 'Dice',
            inAggr: 'TimeSeries2',
            lowerBound: 1,
            upperBound: 7,
            bins: 6,
            addNegInf: false,
            addPosInf: false
        };

        hist2 = base.store("Dice").addStreamAggr(aggrJson);
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
            degreesOfFreedom: 5
        }
        var chi = store.addStreamAggr(aggr);

        // add some values (throwing a pair of dice)
        store.push({ Time: '2015-06-10T14:13:30.0', Sample1: 2 , Sample2: 4 });
        store.push({ Time: '2015-06-10T14:13:31.0', Sample1: 2 , Sample2: 5 });
        store.push({ Time: '2015-06-10T14:13:32.0', Sample1: 2 , Sample2: 2 });
        store.push({ Time: '2015-06-10T14:13:33.0', Sample1: 2 , Sample2: 2 });
        store.push({ Time: '2015-06-10T14:13:34.0', Sample1: 6 , Sample2: 2 });
        store.push({ Time: '2015-06-10T14:13:35.0', Sample1: 3 , Sample2: 5 });
        store.push({ Time: '2015-06-10T14:13:41.0', Sample1: 2 , Sample2: 3 });
        store.push({ Time: '2015-06-10T14:13:41.0', Sample1: 6 , Sample2: 6 });
        store.push({ Time: '2015-06-10T14:13:42.0', Sample1: 5 , Sample2: 1 });
        store.push({ Time: '2015-06-10T14:13:43.0', Sample1: 2 , Sample2: 3 });
        store.push({ Time: '2015-06-10T14:13:44.0', Sample1: 1 , Sample2: 4 });
        store.push({ Time: '2015-06-10T14:13:45.0', Sample1: 3 , Sample2: 2 });
        store.push({ Time: '2015-06-10T14:13:50.0', Sample1: 6 , Sample2: 4 });
        store.push({ Time: '2015-06-10T14:13:51.0', Sample1: 3 , Sample2: 1 });
        store.push({ Time: '2015-06-10T14:13:52.0', Sample1: 4 , Sample2: 1 });
        store.push({ Time: '2015-06-10T14:13:53.0', Sample1: 5 , Sample2: 1 });
        store.push({ Time: '2015-06-10T14:13:54.0', Sample1: 5 , Sample2: 3 });
        store.push({ Time: '2015-06-10T14:13:55.0', Sample1: 4 , Sample2: 1 });

        assert(Math.abs(chi.getFloat() - 4.4666) < 0.001);
        // todo assert correct result
    });

    it('should reset an chi square test aggregator', function () {

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

        // add some values (throwing a pair of dice)
        store.push({ Time: '2015-06-10T14:13:30.0', Sample1: 2, Sample2: 4 });
        store.push({ Time: '2015-06-10T14:13:31.0', Sample1: 2, Sample2: 5 });
        store.push({ Time: '2015-06-10T14:13:32.0', Sample1: 2, Sample2: 2 });
        store.push({ Time: '2015-06-10T14:13:33.0', Sample1: 2, Sample2: 2 });
        store.push({ Time: '2015-06-10T14:13:34.0', Sample1: 6, Sample2: 2 });
        store.push({ Time: '2015-06-10T14:13:35.0', Sample1: 3, Sample2: 5 });
        store.push({ Time: '2015-06-10T14:13:41.0', Sample1: 2, Sample2: 3 });
        store.push({ Time: '2015-06-10T14:13:41.0', Sample1: 6, Sample2: 6 });
        store.push({ Time: '2015-06-10T14:13:42.0', Sample1: 5, Sample2: 1 });
        store.push({ Time: '2015-06-10T14:13:43.0', Sample1: 2, Sample2: 3 });
        store.push({ Time: '2015-06-10T14:13:44.0', Sample1: 1, Sample2: 4 });
        store.push({ Time: '2015-06-10T14:13:45.0', Sample1: 3, Sample2: 2 });
        store.push({ Time: '2015-06-10T14:13:50.0', Sample1: 6, Sample2: 4 });
        store.push({ Time: '2015-06-10T14:13:51.0', Sample1: 3, Sample2: 1 });
        store.push({ Time: '2015-06-10T14:13:52.0', Sample1: 4, Sample2: 1 });
        store.push({ Time: '2015-06-10T14:13:53.0', Sample1: 5, Sample2: 1 });
        store.push({ Time: '2015-06-10T14:13:54.0', Sample1: 5, Sample2: 3 });
        store.push({ Time: '2015-06-10T14:13:55.0', Sample1: 4, Sample2: 1 });

        var endVal = chi.getFloat();

        var fout = qm.fs.openWrite("aggr.tmp");
        chi.save(fout);
        fout.close();

        store.resetStreamAggregates();

        var fin = qm.fs.openRead("aggr.tmp");
        chi.load(fin);
        fin.close();

        assert(chi.getFloat() == endVal);

    });

});


describe('ChiSquare Tests - sfloat', function () {
    var base = undefined;
    var store = undefined;
    var hist1 = undefined;
    var hist2 = undefined;
    beforeEach(function () {
        // create a base with a simple store
        // the store records results of throwing two independent fair dices
        base = new qm.Base({
            mode: "createClean",
            schema: [
            {
                name: "Dice",
                fields: [
                    { name: "Sample1", type: "sfloat" },
                    { name: "Sample2", type: "sfloat" },
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
            value: 'Sample1',
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
            value: 'Sample2',
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
            upperBound: 7,
            bins: 6,
            addNegInf: false,
            addPosInf: false
        };

        hist1 = base.store("Dice").addStreamAggr(aggrJson);

        // add a histogram aggregator, that is connected with the 'TimeSeries2' aggregator
        var aggrJson = {
            name: 'Histogram2',
            type: 'onlineHistogram',
            store: 'Dice',
            inAggr: 'TimeSeries2',
            lowerBound: 1,
            upperBound: 7,
            bins: 6,
            addNegInf: false,
            addPosInf: false
        };

        hist2 = base.store("Dice").addStreamAggr(aggrJson);
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
            degreesOfFreedom: 5
        }
        var chi = store.addStreamAggr(aggr);

        // add some values (throwing a pair of dice)
        store.push({ Time: '2015-06-10T14:13:30.0', Sample1: 2 , Sample2: 4 });
        store.push({ Time: '2015-06-10T14:13:31.0', Sample1: 2 , Sample2: 5 });
        store.push({ Time: '2015-06-10T14:13:32.0', Sample1: 2 , Sample2: 2 });
        store.push({ Time: '2015-06-10T14:13:33.0', Sample1: 2 , Sample2: 2 });
        store.push({ Time: '2015-06-10T14:13:34.0', Sample1: 6 , Sample2: 2 });
        store.push({ Time: '2015-06-10T14:13:35.0', Sample1: 3 , Sample2: 5 });
        store.push({ Time: '2015-06-10T14:13:41.0', Sample1: 2 , Sample2: 3 });
        store.push({ Time: '2015-06-10T14:13:41.0', Sample1: 6 , Sample2: 6 });
        store.push({ Time: '2015-06-10T14:13:42.0', Sample1: 5 , Sample2: 1 });
        store.push({ Time: '2015-06-10T14:13:43.0', Sample1: 2 , Sample2: 3 });
        store.push({ Time: '2015-06-10T14:13:44.0', Sample1: 1 , Sample2: 4 });
        store.push({ Time: '2015-06-10T14:13:45.0', Sample1: 3 , Sample2: 2 });
        store.push({ Time: '2015-06-10T14:13:50.0', Sample1: 6 , Sample2: 4 });
        store.push({ Time: '2015-06-10T14:13:51.0', Sample1: 3 , Sample2: 1 });
        store.push({ Time: '2015-06-10T14:13:52.0', Sample1: 4 , Sample2: 1 });
        store.push({ Time: '2015-06-10T14:13:53.0', Sample1: 5 , Sample2: 1 });
        store.push({ Time: '2015-06-10T14:13:54.0', Sample1: 5 , Sample2: 3 });
        store.push({ Time: '2015-06-10T14:13:55.0', Sample1: 4 , Sample2: 1 });

        assert(Math.abs(chi.getFloat() - 4.4666) < 0.001);
        // todo assert correct result
    });

    it('should reset an chi square test aggregator', function () {

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

        // add some values (throwing a pair of dice)
        store.push({ Time: '2015-06-10T14:13:30.0', Sample1: 2, Sample2: 4 });
        store.push({ Time: '2015-06-10T14:13:31.0', Sample1: 2, Sample2: 5 });
        store.push({ Time: '2015-06-10T14:13:32.0', Sample1: 2, Sample2: 2 });
        store.push({ Time: '2015-06-10T14:13:33.0', Sample1: 2, Sample2: 2 });
        store.push({ Time: '2015-06-10T14:13:34.0', Sample1: 6, Sample2: 2 });
        store.push({ Time: '2015-06-10T14:13:35.0', Sample1: 3, Sample2: 5 });
        store.push({ Time: '2015-06-10T14:13:41.0', Sample1: 2, Sample2: 3 });
        store.push({ Time: '2015-06-10T14:13:41.0', Sample1: 6, Sample2: 6 });
        store.push({ Time: '2015-06-10T14:13:42.0', Sample1: 5, Sample2: 1 });
        store.push({ Time: '2015-06-10T14:13:43.0', Sample1: 2, Sample2: 3 });
        store.push({ Time: '2015-06-10T14:13:44.0', Sample1: 1, Sample2: 4 });
        store.push({ Time: '2015-06-10T14:13:45.0', Sample1: 3, Sample2: 2 });
        store.push({ Time: '2015-06-10T14:13:50.0', Sample1: 6, Sample2: 4 });
        store.push({ Time: '2015-06-10T14:13:51.0', Sample1: 3, Sample2: 1 });
        store.push({ Time: '2015-06-10T14:13:52.0', Sample1: 4, Sample2: 1 });
        store.push({ Time: '2015-06-10T14:13:53.0', Sample1: 5, Sample2: 1 });
        store.push({ Time: '2015-06-10T14:13:54.0', Sample1: 5, Sample2: 3 });
        store.push({ Time: '2015-06-10T14:13:55.0', Sample1: 4, Sample2: 1 });

        var endVal = chi.getFloat();

        var fout = qm.fs.openWrite("aggr.tmp");
        chi.save(fout);
        fout.close();

        store.resetStreamAggregates();

        var fin = qm.fs.openRead("aggr.tmp");
        chi.load(fin);
        fin.close();

        assert(chi.getFloat() == endVal);

    });

});

describe('Stream aggregate set tests', function () {
    var base = undefined;
    var store = undefined;

    beforeEach(function () {
        // create a base with a simple store
        base = new qm.Base({
            mode: "createClean",
            schema: [
            {
                name: "Test",
                fields: [
                    { "name": "Value", "type": "float" },
                    { "name": "Date", "type": "datetime" }
                ]
            }]
        });
        store = base.store('Test');
    });
    afterEach(function () {
        base.close();
    });

    it('Creating simple MA pipeline', function () {
        var tick = new qm.StreamAggr(base, { type: "timeSeriesWinBuf", store: "Test", timestamp: "Date", value: "Value", winsize: 5000 });
        var ma = new qm.StreamAggr(base, { type: "ma", inAggr: tick.name });
        var set = store.addStreamAggr({ type: "set", aggregates: [tick.name, ma.name] });

        assert.throws(function () { store.addStreamAggr({ type: "set", aggregates: [ { a: 1 } ] }); });
        assert.throws(function () { store.addStreamAggr({ type: "set", aggregates: [ store ] }); });

        var time = new Date('2015-06-10T14:13:45.0').getTime();
        store.push({ Value: 1, Date: new Date(time + 1000).toISOString() });
        assert.equal(ma.getFloat(), 1);
        store.push({ Value: 2, Date: new Date(time + 2000).toISOString() });
        assert.equal(ma.getFloat(), 1.5);
        store.push({ Value: 3, Date: new Date(time + 3000).toISOString() });
        assert.equal(ma.getFloat(), 2);
    });

    it('Creating simple MA pipeline (by reference)', function () {
        var tick = new qm.StreamAggr(base, { type: "timeSeriesWinBuf", store: "Test", timestamp: "Date", value: "Value", winsize: 5000 });
        var ma = new qm.StreamAggr(base, { type: "ma", inAggr: tick });
        var set = store.addStreamAggr({ type: "set", aggregates: [tick, ma] });

        assert.throws(function () { store.addStreamAggr({ type: "set", aggregates: [ { a: 1 } ] }); });
        assert.throws(function () { store.addStreamAggr({ type: "set", aggregates: [ store ] }); });

        var time = new Date('2015-06-10T14:13:45.0').getTime();
        store.push({ Value: 1, Date: new Date(time + 1000).toISOString() });
        assert.equal(ma.getFloat(), 1);
        store.push({ Value: 2, Date: new Date(time + 2000).toISOString() });
        assert.equal(ma.getFloat(), 1.5);
        store.push({ Value: 3, Date: new Date(time + 3000).toISOString() });
        assert.equal(ma.getFloat(), 2);
    });

    it('Creating simple MA pipeline (combination of name and reference)', function () {
        var tick = new qm.StreamAggr(base, { type: "timeSeriesWinBuf", store: "Test", timestamp: "Date", value: "Value", winsize: 5000 });
        var ma = new qm.StreamAggr(base, { type: "ma", inAggr: tick });
        var set = store.addStreamAggr({ type: "set", aggregates: [tick, ma] });

        assert.throws(function () { store.addStreamAggr({ type: "set", aggregates: [ { a: 1 } ] }); });
        assert.throws(function () { store.addStreamAggr({ type: "set", aggregates: [ store ] }); });

        var time = new Date('2015-06-10T14:13:45.0').getTime();
        store.push({ Value: 1, Date: new Date(time + 1000).toISOString() });
        assert.equal(ma.getFloat(), 1);
        store.push({ Value: 2, Date: new Date(time + 2000).toISOString() });
        assert.equal(ma.getFloat(), 1.5);
        store.push({ Value: 3, Date: new Date(time + 3000).toISOString() });
        assert.equal(ma.getFloat(), 2);
    });
});

describe('Online histogram tests', function () {
    var base = undefined;
    var store = undefined;

    beforeEach(function () {
        // create a base with a simple store
        base = new qm.Base({
            mode: "createClean",
            schema: [
            {
                name: "Store",
                fields: [
                    { "name": "Value", "type": "float" },
                    { "name": "Time", "type": "datetime" }
                ]
            }]
        });
        store = base.store("Store");
    });
    afterEach(function () {
        base.close();
    });

    it('Tests online histogram increment/decrement', function () {

        var winbuf = store.addStreamAggr({
            type: 'timeSeriesWinBuf',
            timestamp: 'Time',
            value: 'Value',
            winsize: 1000
        });

        var hist = store.addStreamAggr({
            type: 'onlineHistogram',
            inAggr: winbuf.name,
            lowerBound: 0,
            upperBound: 5,
            bins: 5,
            addNegInf: false,
            addPosInf: false
        });

        function pushDatIncrTs(val, ts, incr) {
            ts = ts + incr;
            store.push({ Value: val, Time: ts });
            return ts;
        }
        function assertEqualArray(ar1, ar2) {
            assert(ar1.length, ar2.length);
            for (var i = 0; i < ar1.length; i++) {
                assert.equal(ar1[i], ar2[i]);
            }

        }

        var ts = new Date().getTime();
        ts = pushDatIncrTs(0.1, ts, 1);
        ts = pushDatIncrTs(1.1, ts, 1);
        ts = pushDatIncrTs(2.1, ts, 1);
        ts = pushDatIncrTs(3.1, ts, 1);
        ts = pushDatIncrTs(5, ts, 1);
        ts = pushDatIncrTs(5.1, ts, 1);
        assertEqualArray(hist.val.counts, [1, 1, 1, 1, 1]);
        ts = pushDatIncrTs(1, ts, 1001);
        assertEqualArray(hist.val.counts, [0, 1, 0, 0, 0]);
        ts = pushDatIncrTs(1, ts, 1000);
        assertEqualArray(hist.val.counts, [0, 2, 0, 0, 0]);
    });

    it('Tests online histogram reset/save/load', function () {

        var winbuf = store.addStreamAggr({
            type: 'timeSeriesWinBuf',
            timestamp: 'Time',
            value: 'Value',
            winsize: 1000
        });

        var hist = store.addStreamAggr({
            type: 'onlineHistogram',
            inAggr: winbuf.name,
            lowerBound: 0,
            upperBound: 5,
            bins: 5,
            addNegInf: false,
            addPosInf: false
        });

        function pushDatIncrTs(val, ts, incr) {
            ts = ts + incr;
            store.push({ Value: val, Time: ts });
            return ts;
        }
        function assertEqualArray(ar1, ar2) {
            assert(ar1.length, ar2.length);
            for (var i = 0; i < ar1.length; i++) {
                assert.equal(ar1[i], ar2[i]);
            }

        }

        var ts = new Date().getTime();
        ts = pushDatIncrTs(0.1, ts, 1);
        ts = pushDatIncrTs(1.1, ts, 1);
        ts = pushDatIncrTs(2.1, ts, 1);
        ts = pushDatIncrTs(3.1, ts, 1);
        ts = pushDatIncrTs(5, ts, 1);
        ts = pushDatIncrTs(5.1, ts, 1);
        ts = pushDatIncrTs(1, ts, 1001);
        ts = pushDatIncrTs(1, ts, 1000);

        var fout = qm.fs.openWrite('onlineHist.bin');
        hist.save(fout);
        hist.reset();
        assertEqualArray(hist.val.counts, [0, 0, 0, 0, 0]);
        var fin = qm.fs.openRead('onlineHist.bin');
        hist.load(fin);
        assertEqualArray(hist.val.counts, [0, 2, 0, 0, 0]);
    });

    it('Tests online histogram autoResize', function () {

        var winbuf = store.addStreamAggr({
            type: 'timeSeriesWinBuf',
            timestamp: 'Time',
            value: 'Value',
            winsize: 1000
        });

        var hist = store.addStreamAggr({
            type: 'onlineHistogram',
            inAggr: winbuf.name,
            lowerBound: 0,
            upperBound: 5,
            bins: 5,
            addNegInf: false,
            addPosInf: false,
            autoResize: true
        });

        function pushDatIncrTs(val, ts, incr) {
            ts = ts + incr;
            store.push({ Value: val, Time: ts });
            return ts;
        }
        function assertEqualArray(ar1, ar2) {
            assert(ar1.length, ar2.length);
            for (var i = 0; i < ar1.length; i++) {
                assert.equal(ar1[i], ar2[i]);
            }

        }

        var t = new Date().getTime();
        t = pushDatIncrTs(1.1, t, 1);
        assertEqualArray(hist.val.counts, [1]);
        assertEqualArray(hist.val.bounds, [1, 2]);
        t = pushDatIncrTs(0.1, t, 1);
        t = pushDatIncrTs(2.1, t, 1);
        t = pushDatIncrTs(3.1, t, 1);
        assertEqualArray(hist.val.counts, [1, 1, 1, 1]);
        assertEqualArray(hist.val.bounds, [0, 1, 2, 3, 4]);
        t = pushDatIncrTs(1, t, 1001);
        assertEqualArray(hist.val.counts, [0, 1, 0, 0]);
        t = pushDatIncrTs(5, t, 1001);
        assertEqualArray(hist.val.counts, [0, 0, 0, 0, 1]);

    });

    it('Tests online histogram autoResize with -inf and inf included as bounds', function () {
        var winbuf = store.addStreamAggr({
            type: 'timeSeriesWinBuf',
            timestamp: 'Time',
            value: 'Value',
            winsize: 1000
        });

        var hist = store.addStreamAggr({
            type: 'onlineHistogram',
            inAggr: winbuf.name,
            lowerBound: 0,
            upperBound: 5,
            bins: 5,
            addNegInf: true,
            addPosInf: true,
            autoResize: true
        });

        function pushDatIncrTs(val, ts, incr) {
            ts = ts + incr;
            store.push({ Value: val, Time: ts });
            return ts;
        }
        function assertEqualArray(ar1, ar2) {
            assert(ar1.length, ar2.length);
            for (var i = 0; i < ar1.length; i++) {
                assert.equal(ar1[i], ar2[i]);
            }

        }

        var t = new Date().getTime();
        t = pushDatIncrTs(1.1, t, 1);
        assertEqualArray(hist.val.counts, [0, 1, 0]);
        assertEqualArray(hist.val.bounds.slice(1,3), [1, 2]);
        t = pushDatIncrTs(0.1, t, 1);
        t = pushDatIncrTs(2.1, t, 1);
        t = pushDatIncrTs(3.1, t, 1);
        assertEqualArray(hist.val.counts, [0, 1, 1, 1, 1, 0]);
        assertEqualArray(hist.val.bounds.slice(1,6), [0, 1, 2, 3, 4]);
        t = pushDatIncrTs(1, t, 1001);
        assertEqualArray(hist.val.counts, [0, 0, 1, 0, 0, 0]);
        t = pushDatIncrTs(5, t, 1001);
        assertEqualArray(hist.val.counts, [0, 0, 0, 0, 0, 1, 0]);
        t = pushDatIncrTs(5.1, t, 1001);
        assertEqualArray(hist.val.counts, [0, 0, 0, 0, 0, 0, 1]);
        t = pushDatIncrTs(-5.1, t, 1001);
        assertEqualArray(hist.val.counts, [1, 0, 0, 0, 0, 0, 0]);
        t = pushDatIncrTs(-5.1, t, 1001);
        t = pushDatIncrTs(100, t, 1);
        t = pushDatIncrTs(5, t, 1);
        t = pushDatIncrTs(1.5, t, 1);
        assertEqualArray(hist.val.counts, [1, 0, 1, 0, 0, 1, 1]);
    });
});

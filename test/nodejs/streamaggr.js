var qm = require('qminer');
var assert = require('assert');


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
                      { name: 'Gendre', type: 'string'}
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
                    console.log('updated stream aggr for: ', rec.Name);
                    data = rec.Name.length;
                };
                this.saveJson = function (limit) {
                    return { val: data };
                };
            });


            var id1 = base.store('People').push({ Name: "John", Gendre: "Male" });
            assert(s.saveJson().val == 4);

            var id2 = base.store('People').push({ Name: "Mary", Gendre: "Female" });
            assert(s.saveJson().val == 4);

            var id3 = base.store('People').push({ Name: "Jim", Gendre: "Male" });
            assert(s.saveJson().val == 3);
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
    describe.only('OnAdd Tests', function () {
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
    })
})




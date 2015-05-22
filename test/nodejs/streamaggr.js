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
                  fields: [{ name: 'name', type: 'string' }],
              }
            ]
        });

        store = base.store('People');

    });
    afterEach(function () {
        base.close();
    });

    describe('JsStreamAggr Test', function () {
        it('should register a Js extractor, which counts record.name string length', function () {

            var s = store.addStreamAggr(new function () {
                var data = 0;
                this.name = 'anomaly';
                this.onAdd = function (rec) {
                    console.log('updated stream aggr for: ', rec.name);
                    data = rec.name.length;
                };
                this.saveJson = function (limit) {
                    return { val: data };
                };
            });


            var id1 = base.store('People').add({ name: "John" });
            assert(s.saveJson().val == 4);

            var id2 = base.store('People').add({ name: "Mary" });
            assert(s.saveJson().val == 4);

            var id3 = base.store('People').add({ name: "Jim" });
            assert(s.saveJson().val == 3);
        })
    })
})




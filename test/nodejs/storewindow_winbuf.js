var assert = require('../../src/nodejs/scripts/assert.js');
var qm = require('../../index.js');

describe('Windowed store and window buffer', function () {
    var base = undefined;
    var store = undefined;
    beforeEach(function () {
        base = new qm.Base({
            mode: 'createClean',
            schema: [{
                name: 'Function',
                fields: [
                    { name: 'Time', type: 'datetime' },
                    { name: 'X', type: 'float' },
                    { name: 'Y', type: 'float' }
                ],
                window: 3
            }]
        });
        store = base.store('Function');
    });
    afterEach(function () {
        base.close();
    });

    describe('Invalid record ID in the buffer', function () {
        it('should throw', function () {
            var winX = store.addStreamAggr({
                type: 'timeSeriesWinBuf',
                timestamp: 'Time',
                value: 'X',
                winsize: 1000
            });

            store.push({ Time: '2015-06-10T14:13:32.001', X: 0, Y: -2 });
            store.push({ Time: '2015-06-10T14:13:32.002', X: 0, Y: -1 });
            store.push({ Time: '2015-06-10T14:13:32.003', X: 0, Y: 1 });
            store.push({ Time: '2015-06-10T14:13:32.004', X: 0, Y: 2 });

            base.garbageCollect();

			assert.throws(function () {
                var valvec = winX.getValueVector();
            });
        });
    });
});
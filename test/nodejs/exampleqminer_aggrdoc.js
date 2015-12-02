require('qminer').la.Vector.prototype.print = function () { };require('qminer').la.SparseVector.prototype.print = function () { };require('qminer').la.SparseMatrix.prototype.print = function () { };require('qminer').la.Matrix.prototype.print = function () { };describe('example tests for the qminer_aggrdoc.js file', function () {
describe("Qminer module, number 1", function () {
it("should make test number 1", function () {

 // import module
 var qm = require('qminer');

});
});
describe("Stream Aggregat, number 2", function () {
it("should make test number 2", function () {

 // import qm module
 var qm = require('qminer');
 // create a simple base containing one store
 var base = new qm.Base({
    mode: "createClean",
    schema: [{
        name: "People",
        fields: [
            { name: "Name", type: "string" },
            { name: "Gendre", type: "string" },
        ]
    },
    {
        name: "Laser",
        fields: [
            { name: "Time", type: "datetime" },
            { name: "WaveLength", type: "float" }
        ]
    }]
 });

 // create a new stream aggregator for 'People' store, get the length of the record name (with the function object)
 var aggr = new qm.StreamAggr(base, new function () {
    var length = 0;
    this.name = 'nameLength',
    this.onAdd = function (rec) {
        length = rec.Name.length;
    };
    this.saveJson = function (limit) {
        return { val: length };
    }
 }, "People");

 // create a new time series window buffer stream aggregator for 'Laser' store (with the JSON object)
 var wavelength = {
     name: "WaveLengthLaser",
     type: "timeSeriesWinBuf",
     store: "Laser",
     timestamp: "Time",
     value: "WaveLength",
     winsize: 10000
 }
 var sa = base.store("Laser").addStreamAggr(wavelength);
 base.close();

});
});
describe("@typedef {module:qm.StreamAggr} StreamAggregateTimeSeriesWindo, number 3", function () {
it("should make test number 3", function () {
 
 // import the qm module
 var qm = require('qminer');
 // create a base with a simple store
 var base = new qm.Base({
    mode: "createClean",
    schema: [
    {
        name: "Heat",
        fields: [
            { name: "Celcius", type: "float" },
            { name: "Time", type: "datetime" }
        ]
    }]
 });

 // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'Celcius' field
 // and the timestamp from the 'Time' field. The size of the window is 2 seconds (2000ms).
 var aggr = {
    name: 'TimeSeriesAggr',
    type: 'timeSeriesWinBuf',
    store: 'Heat',
    timestamp: 'Time',
    value: 'Celcius',
    winsize: 2000
 };
 base.store("Heat").addStreamAggr(aggr); 
 base.close();

});
});
describe("@typedef {module:qm.StreamAggr} StreamAggregateRecordBuffe, number 4", function () {
it("should make test number 4", function () {

 // import the qm module
 var qm = require('qminer');
 // create a base with a simple store
 var base = new qm.Base({
    mode: "createClean",
    schema: [
    {
        name: "Heat",
        fields: [
            { name: "Celcius", type: "float" },
            { name: "Time", type: "datetime" }
        ]
    }]
 });

 // create a new time series stream aggregator for the 'Heat' store. The size of the window is 3 records.
 var aggr = {
    name: 'Delay',
    type: 'recordBuffer',
    size: 3
 };
 base.store("Heat").addStreamAggr(aggr);
 base.close();

});
});
describe("@typedef {module:qm.StreamAggr} StreamAggregateSu, number 5", function () {
it("should make test number 5", function () {

 // import the qm module
 var qm = require('qminer');
 // create a base with a simple store
 var base = new qm.Base({
    mode: "createClean",
    schema: [
    {
        name: "Income",
        fields: [
            { name: "Amount", type: "float" },
            { name: "Time", type: "datetime" }
        ]
    }]
 });

 // create a new time series stream aggregator for the 'Income' store, that takes the values from the 'Amount' field
 // and the timestamp from the 'Time' field. The size of the window should 1 week.
 var timeser = {
    name: 'TimeSeriesAggr',
    type: 'timeSeriesWinBuf',
    store: 'Income',
    timestamp: 'Time',
    value: 'Amount',
    winsize: 604800000 // 7 days in miliseconds
 };
 var timeSeries = base.store("Income").addStreamAggr(timeser);

 // add a sum aggregator, that is connected with the 'TimeSeriesAggr' aggregator
 var sum = {
    name: 'SumAggr',
    type: 'winBufSum',
    store: 'Heat',
    inAggr: 'TimeSeriesAggr'
 };
 var sumAggr = base.store("Income").addStreamAggr(sum);
 base.close();

});
});
describe("@typedef {module:qm.StreamAggr} StreamAggregateMi, number 6", function () {
it("should make test number 6", function () {

 // import the qm module
 var qm = require('qminer');
 // create a base with a simple store
 var base = new qm.Base({
    mode: "createClean",
    schema: [
    {
        name: "Heat",
        fields: [
            { name: "Celcius", type: "float" },
            { name: "Time", type: "datetime" }
        ]
    }]
 });

 // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'Celcius' field
 // and the timestamp from the 'Time' field. The size of the window is 1 day.
 var timeser = {
    name: 'TimeSeriesAggr',
    type: 'timeSeriesWinBuf',
    store: 'Heat',
    timestamp: 'Time',
    value: 'Celcius',
    winsize: 86400000 // 1 day in miliseconds
 };
 var timeSeries = base.store("Heat").addStreamAggr(timeser);

 // add a min aggregator, that is connected with the 'TimeSeriesAggr' aggregator
 var min = {
    name: 'MinAggr',
    type: 'winBufMin',
    store: 'Heat',
    inAggr: 'TimeSeriesAggr'
 };
 var minimal = base.store("Heat").addStreamAggr(min);
 base.close();

});
});
describe("@typedef {module:qm.StreamAggr} StreamAggregateMa, number 7", function () {
it("should make test number 7", function () {

 // import the qm module
 var qm = require('qminer');
 // create a base with a simple store
 var base = new qm.Base({
    mode: "createClean",
    schema: [
    {
        name: "Heat",
        fields: [
            { name: "Celcius", type: "float" },
            { name: "Time", type: "datetime" }
        ]
    }]
 });

 // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'Celcius' field
 // and the timestamp from the 'Time' field. The size of the window is 1 day.
 var timeser = {
    name: 'TimeSeriesAggr',
    type: 'timeSeriesWinBuf',
    store: 'Heat',
    timestamp: 'Time',
    value: 'Celcius',
    winsize: 86400000 // one day in miliseconds
 };
 var timeSeries = base.store("Heat").addStreamAggr(timeser);

 // add a max aggregator, that is connected with the 'TimeSeriesAggr' aggregator
 var max = {
    name: 'MaxAggr',
    type: 'winBufMax',
    store: 'Heat',
    inAggr: 'TimeSeriesAggr'
 };
 var maximal = base.store("Heat").addStreamAggr(max);
 base.close();

});
});
describe("@typedef {module:qm.StreamAggr} StreamAggregateTimeSeriesTic, number 8", function () {
it("should make test number 8", function () {

 // import the qm module
 var qm = require('qminer');
 // create a base with a simple store
 var base = new qm.Base({
    mode: "createClean",
    schema: [
    {
        name: "Students",
        fields: [
            { name: "Id", type: "float" },
            { name: "TimeOfGraduation", type: "datetime" }
        ]
    }]
 });

 // create a new time series tick stream aggregator for the 'Students' store, that takes the values from the 'Id' field
 // and the timestamp from the 'TimeOfGraduation' field.
 var tick = {
    name: 'TimeSeriesTickAggr',
    type: 'timeSeriesTick',
    store: 'Students',
    timestamp: 'TimeOfGraduation',
    value: 'Id',
 };
 var timeSeriesTick = base.store("Students").addStreamAggr(tick);
 base.close();

});
});
describe("@typedef {module:qmStreamAggr} StreamAggregateMovingAverag, number 9", function () {
it("should make test number 9", function () {

 // import the qm module
 var qm = require('qminer');
 // create a base with a simple store
 var base = new qm.Base({
    mode: "createClean",
    schema: [
    {
        name: "Heat",
        fields: [
            { name: "Celcius", type: "float" },
            { name: "Time", type: "datetime" }
        ]
    }]
 });

 // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'Celcius' field
 // and the timestamp from the 'Time' field. The size of the window should be 1 day.
 var timeser = {
    name: 'TimeSeriesAggr',
    type: 'timeSeriesWinBuf',
    store: 'Heat',
    timestamp: 'Time',
    value: 'Celcius',
    winsize: 86400000
 };
 var timeSeries = base.store("Heat").addStreamAggr(timeser);

 // add a moving average aggregator, that is connected with the 'TimeSeriesAggr' aggregator
 var ma = {
    name: 'movingAverageAggr',
    type: 'ma',
    store: 'Heat',
    inAggr: 'TimeSeriesAggr'
 };
 var movingAverage = base.store("Heat").addStreamAggr(ma);
 base.close();

});
});
describe("@typedef {module:qmStreamAggr} StreamAggregateEM, number 10", function () {
it("should make test number 10", function () {

 // import the qm module
 var qm = require('qminer');
 // create a base with a simple store
 var base = new qm.Base({
    mode: "createClean",
    schema: [
    {
        name: "Heat",
        fields: [
            { name: "Celcius", type: "float" },
            { name: "Time", type: "datetime" }
        ]
    }]
 });

 // create a new time series tick stream aggregator for the 'Heat' store, that takes the values from the 'Celcius' field
 // and the timestamp from the 'Time' field. The size of the window should be 1 hour.
 var timeser = {
    name: 'TimeSeriesAggr',
    type: 'timeSeriesTick',
    store: 'Heat',
    timestamp: 'Time',
    value: 'Celcius',
    winsize: 3600000
 };
 var timeSeries = base.store("Heat").addStreamAggr(timeser);

 // add an exponentional moving average aggregator, that is connected with the 'TimeSeriesAggr' aggregator.
 // It should interpolate with the previous value, the decay should be 3 seconds and the initWindow should be 2 seconds.
 var ema = {
    name: 'emaAggr',
    type: 'ema',
    store: 'Heat',
    inAggr: 'TimeSeriesAggr',
    emaType: 'previous',
    interval: 3000,
    initWindow: 2000
 };
 var expoMovingAverage = base.store("Heat").addStreamAggr(ema);
 base.close();

});
});
describe("@typedef {module:qm.StreamAggr} StreamAggregateMovingVarianc, number 11", function () {
it("should make test number 11", function () {

 // import the qm module
 var qm = require('qminer');
 // create a base with a simple store
 var base = new qm.Base({
    mode: "createClean",
    schema: [
    {
        name: "Heat",
        fields: [
            { name: "Celcius", type: "float" },
            { name: "Time", type: "datetime" }
        ]
    }]
 });

 // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'Celcius' field
 // and the timestamp from the 'Time' field. The size of the window is 1 day.
 var timeser = {
    name: 'TimeSeriesAggr',
    type: 'timeSeriesWinBuf',
    store: 'Heat',
    timestamp: 'Time',
    value: 'Celcius',
    winsize: 86400000
 };
 var timeSeries = base.store("Heat").addStreamAggr(timeser);

 // add a variance aggregator, that is connected with the 'TimeSeriesAggr' aggregator
 var variance = {
    name: 'varAggr',
    type: 'variance',
    store: 'Heat',
    inAggr: 'TimeSeriesAggr'
 };
 var varianceAggr = base.store("Heat").addStreamAggr(variance);
 base.close();

});
});
describe("@typedef {module:qm.StreamAggr} StreamAggregateMovingCovarianc, number 12", function () {
it("should make test number 12", function () {

 // import the qm module
 var qm = require('qminer');
 // create a base with a simple store
 var base = new qm.Base({
    mode: "createClean",
    schema: [
    {
        name: "Heat",
        fields: [
            { name: "Celcius", type: "float" },
            { name: "WaterConsumption", type: "float" },
            { name: "Time", type: "datetime" }
        ]
    }]
 });

 // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'Celcius' field
 // and the timestamp from the 'Time' field. The size of the window is 1 day.
 var celcius = {
    name: 'CelciusAggr',
    type: 'timeSeriesWinBuf',
    store: 'Heat',
    timestamp: 'Time',
    value: 'Celcius',
    winsize: 86400000
 }; base.store("Heat").addStreamAggr(celcius);

 // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'WaterConsumption' field
 // and the timestamp from the 'Time' field. The size of the window is 1 day.
 var water = {
    name: 'WaterAggr',
    type: 'timeSeriesWinBuf',
    store: 'Heat',
    timestamp: 'Time',
    value: 'WaterConsumption',
    winsize: 86400000
 }; base.store("Heat").addStreamAggr(water);

 // add a covariance aggregator, that is connected with the 'CelciusAggr' and 'WaterAggr' stream aggregators
 var covariance = {
    name: 'covAggr',
    type: 'covariance',
    store: 'Heat',
    inAggrX: 'CelciusAggr',
    inAggrY: 'WaterAggr'
 };
 var covarianceAggr = base.store("Heat").addStreamAggr(covariance);
 base.close();

});
});
describe("@typedef {module:qm.StreamAggr} StreamAggregateMovingCorrelatio, number 13", function () {
it("should make test number 13", function () {

 // import the qm module
 var qm = require('qminer');
 // create a base with a simple store
 var base = new qm.Base({
    mode: "createClean",
    schema: [
    {
        name: "Heat",
        fields: [
            { name: "Celcius", type: "float" },
            { name: "WaterConsumption", type: "float" },
            { name: "Time", type: "datetime" }
        ]
    }]
 });

 // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'Celcius' field
 // and the timestamp from the 'Time' field. The size of the window is 1 day.
 var celcius = {
    name: 'CelciusAggr',
    type: 'timeSeriesWinBuf',
    store: 'Heat',
    timestamp: 'Time',
    value: 'Celcius',
    winsize: 86400000
 }; base.store("Heat").addStreamAggr(celcius);

 // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'WaterConsumption' field
 // and the timestamp from the 'Time' field. The size of the window is 1 day.
 var water = {
    name: 'WaterAggr',
    type: 'timeSeriesWinBuf',
    store: 'Heat',
    timestamp: 'Time',
    value: 'WaterConsumption',
    winsize: 86400000
 }; base.store("Heat").addStreamAggr(water);

 // add a covariance aggregator, that is connected with the 'CelciusAggr' and 'WaterAggr' aggregators
 var covariance = {
    name: 'covarianceAggr',
    type: 'covariance',
    store: 'Heat',
    inAggrX: 'CelciusAggr',
    inAggrY: 'WaterAggr'
 }; base.store("Heat").addStreamAggr(covariance);

 // add the two variance aggregators, that take from the 'Celcius' and 'WaterConsumption' fields, respectively.
 var celVar = {
    name: 'celciusVarAggr',
    type: 'variance',
    store: 'Heat',
    inAggr: 'CelciusAggr'
 }; base.store("Heat").addStreamAggr(celVar);

 var waterVar = {
    name: 'waterVarAggr',
    type: 'variance',
    store: 'Heat',
    inAggr: 'WaterAggr'
 }; base.store("Heat").addStreamAggr(waterVar);

 // add a correlation aggregator, that is connected to 'CovarianceAggr', 'CelciusVarAggr' and 'WaterValAggr' aggregators
 var corr = {
    name: 'corrAggr',
    type: 'correlation',
    store: 'Heat',
    inAggrCov: 'covarianceAggr',
    inAggrVarX: 'celciusVarAggr',
    inAggrVarY: 'waterVarAggr'
 };
 var correlation = base.store("Heat").addStreamAggr(corr);
 base.close();

});
});
describe("@typedef {module:qm.StreamAggr} StreamAggregateResample, number 14", function () {
it("should make test number 14", function () {

 // import the qm module
 var qm = require('qminer');
 // create a base with a simple store
 var base = new qm.Base({
    mode: "createClean",
    schema: [
    {
        name: "Heat",
        fields: [
            { name: "Celcius", type: "float" },
            { name: "Time", type: "datetime" }
        ]
    },
    {
        name: "interpolatedValues",
        fields: [
            { name: "Value", type: "float" },
            { name: "Time", type: "datetime" }
        ]
    }]
 });
 // create a new resampler stream aggregator for the 'Heat' store, that takes the values from the 'Celcius' field
 // and the timestamp from the 'Time' field. The interpolated values are stored in the 'interpolatedValues' store.
 // The interpolation should be linear and the interval should be 2 seconds.
 var res = {
    name: 'resamplerAggr',
    type: 'resampler',
    store: 'Heat',
    outStore: 'interpolatedValues',
    timestamp: 'Time',
    fields: [{
        name: 'Celcius',
        interpolator: 'linear'
    }],
    createStore: false,
    interval: 2000
 };
 var resampler = base.store("Heat").addStreamAggr(res);
 base.close();

});
});
describe("@typedef {module:qm.StreamAggr} StreamAggregateMerge, number 15", function () {
it("should make test number 15", function () {

 // import the qm module
 var qm = require('qminer');
 // create a base with a simple store
 var base = new qm.Base({
    mode: "createClean",
    schema: [
    {
        name: "Cars",
        fields: [
            { name: "NumberOfCars", type: "float" },
            { name: "Time", type: "datetime" }
        ]
    },
    {
        name: "Temperature",
        fields: [
            { name: "Celcius", type: "float" },
            { name: "Time", type: "datetime" }
        ]
    },
    {
        name: "Merged",
        fields: [
            { name: "NumberOfCars", type: "float" },
            { name: "Celcius", type: "float" },
            { name: "Time", type: "datetime" }
        ]
    }]
 });
 // create a new merger stream aggregator that mergers the records of the 'Cars' and 'Temperature' stores.
 // The records are interpolated linearly and stored in the 'Merged' store.
 var mer = {
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
 var merger = new qm.StreamAggr(base, mer);
 base.close();

});
});
describe("@typedef {module:qm.StreamAggr} StreamAggregateHistogra, number 16", function () {
it("should make test number 16", function () {

 // import the qm module
 var qm = require('qminer');
 // create a base with a simple store
 var base = new qm.Base({
    mode: "createClean",
    schema: [
    {
        name: "Heat",
        fields: [
            { name: "Celcius", type: "float" },
            { name: "Time", type: "datetime" }
        ]
    }]
 });

 // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'Celcius' field
 // and the timestamp from the 'Time' field. The size of the window is 1 day.
 var timeser = {
    name: 'TimeSeriesBuffer',
    type: 'timeSeriesWinBuf',
    store: 'Heat',
    timestamp: 'Time',
    value: 'Celcius',
    winsize: 86400000 // one day in miliseconds
 };
 var timeSeries = base.store("Heat").addStreamAggr(timeser);

 // add a histogram aggregator, that is connected with the 'TimeSeriesAggr' aggregator
 var aggrJson = {
    name: 'Histogram',
    type: 'onlineHistogram',
    store: 'Heat',
    inAggr: 'TimeSeriesBuffer',
    lowerBound: 0,
    upperBound: 10,
    bins: 5,
    addNegInf: false,
    addPosInf: false
 };
 var hist = base.store("Heat").addStreamAggr(aggrJson);
 base.close();

});
});
describe("@typedef {module:qm.StreamAggr} StreamAggregateSlottedHistogra, number 17", function () {
it("should make test number 17", function () {

 // import the qm module
 var qm = require('qminer');
 // create a base with a simple store
 var base = new qm.Base({
    mode: "createClean",
    schema: [
    {
        name: "Heat",
        fields: [
            { name: "Celcius", type: "float" },
            { name: "Time", type: "datetime" }
        ]
    }]
 });

 // create a new time series stream aggregator for the 'Heat' store, that takes the values from the 'Celcius' field
 // and the timestamp from the 'Time' field. The size of the window is 4 weeks.
 var timeser = {
    name: 'TimeSeriesBuffer',
    type: 'timeSeriesWinBuf',
    store: 'Heat',
    timestamp: 'Time',
    value: 'Celcius',
    winsize: 2419200000 // 4 weeks
 };
 var timeSeries = base.store("Heat").addStreamAggr(timeser);

 // add a slotted-histogram aggregator, that is connected with the 'TimeSeriesAggr' aggregator
 // it will present accumulated histogram for the last 2 hours (window) of the week (period) for the last 4 weeks (see aggregate above)
 var aggrJson = {
    name: 'Histogram',
    type: 'onlineSlottedHistogram',
    store: 'Heat',
    inAggr: 'TimeSeriesBuffer',
    period: 604800000, // 1 week
    window: 7200000, // 2h
    bins: 5, // 5 possible clusters
    granularity: 300000  // 5 min
 };
 var hist = base.store("Heat").addStreamAggr(aggrJson);
 base.close();

});
});
describe("@typedef {module:qm.StreamAggr} StreamAggregateVecDif, number 18", function () {
it("should make test number 18", function () {

 // import the qm module
 var qm = require('qminer');
 // create a base with a simple store
 // the store records results of clustering
 var base = new qm.Base({
 mode: "createClean",
 schema: [
 {
 	name: "Rpm",
 	fields: [
 		{ name: "ClusterId", type: "float" },
 		{ name: "Time", type: "datetime" }
 	]
 }]
 });		
 
 var store = base.store('Rpm');
 
 // create a new time series stream aggregator for the 'Rpm' store that takes the recorded cluster id
 // and the timestamp from the 'Time' field. The size of the window is 4 weeks.
 var timeser1 = {
 	name: 'TimeSeries1',
 	type: 'timeSeriesWinBuf',
 	store: 'Rpm',
 	timestamp: 'Time',
 	value: 'ClusterId',
 	winsize: 7200000 // 2 hours
 };
 var timeSeries1 = base.store("Rpm").addStreamAggr(timeser1);
 
 // add a histogram aggregator, that is connected with the 'TimeSeries1' aggregator
 var aggrJson1 = {
 	name: 'Histogram1',
 	type: 'onlineHistogram',
 	store: 'Rpm',
 	inAggr: 'TimeSeries1',
 	lowerBound: 0,
 	upperBound: 5,
 	bins: 5,
 	addNegInf: false,
 	addPosInf: false
 };
 var hist1 = base.store("Rpm").addStreamAggr(aggrJson1);
 
 // create a new time series stream aggregator for the 'Rpm' store that takes the recorded cluster id
 // and the timestamp from the 'Time' field. 
 var timeser2 = {
 	name: 'TimeSeries2',
 	type: 'timeSeriesWinBuf',
 	store: 'Rpm',
 	timestamp: 'Time',
 	value: 'ClusterId',
 	winsize: 21600000 // 6 hours
 };
 var timeSeries2 = base.store("Rpm").addStreamAggr(timeser2);
 
 // add a histogram aggregator, that is connected with the 'TimeSeries1' aggregator
 var aggrJson2 = {
 	name: 'Histogram2',
 	type: 'onlineHistogram',
 	store: 'Rpm',
 	inAggr: 'TimeSeries2',
 	lowerBound: 0,
 	upperBound: 5,
 	bins: 5,
 	addNegInf: false,
 	addPosInf: false
 };
 var hist2 = base.store("Rpm").addStreamAggr(aggrJson2);
 
 // add diff aggregator that subtracts Histogram1 with 2h window from Histogram2 with 6h window
 var aggrJson3 = {
 	name: 'DiffAggr',
 	type: 'onlineVecDiff',
 	storeX: 'Rpm',
 	storeY: 'Rpm',
 	inAggrX: 'Histogram2',
 	inAggrY: 'Histogram1'
 }
 var diff = store.addStreamAggr(aggrJson3);
 base.close();

});
});
describe("Returns the value of the specific stream aggregator. For return values see {@link module:qm~StreamAggregators}, number 19", function () {
it("should make test number 19", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a simple base containing one store
	 var base = new qm.Base({
	    mode: 'createClean',
	    schema: [{
	        name: 'Grades',
	        fields: [
	            { name: 'Grade', type: 'int' },
	            { name: 'Procents', type: 'float' },
	            { name: 'Time', type: 'datetime' }
	        ]
	    }]
	 });
	 // create a new time series stream aggregator which takes the values from the 'Procents' field 
	 // and the timestamp from the 'Time' field. The size of the window is 1 year.
	 var ts = {
	    name: 'GradesAggr',
	    type: 'timeSeriesWinBuf',
	    store: 'Grades',
	    timestamp: 'Time',
	    value: 'Procents',
	    winsize: 31536000000 
	 };
	 var timeSeries = base.store('Grades').addStreamAggr(ts);
	 // create a new moving average stream aggregator that takes the values from the 
	 // 'GradesAggr' stream aggregator
	 var ma = {
	    name: 'AverageGrade',
	    type: 'ma',
	    store: 'Grades',
	    inAggr: 'GradesAggr'
	 }
	 var averageGrade = base.store('Grades').addStreamAggr(ma);
	 // add some grades in the 'Grades' store
	 base.store("Grades").push({ Grade: 7, Procents: 65, Time: '2014-11-23T10:00:00.0' });
	 base.store("Grades").push({ Grade: 9, Procents: 88, Time: '2014-12-20T12:00:00.0' });
	 base.store("Grades").push({ Grade: 8, Procents: 70, Time: '2015-02-03T10:00:00.0' });
	 // get the average grade procents by using the getFloat method
	 var average = averageGrade.getFloat(); // returns 74 + 1/3
	 base.close();
	
});
});
describe("Returns the timestamp value of the newest record in buffer, number 20", function () {
it("should make test number 20", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a simple base containing one store
	 var base = new qm.Base({
	    mode: 'createClean',
	    schema: [{
	        name: 'GameCollection',
	        fields: [
	            { name: 'GameName', type: 'string' },
	            { name: 'Price', type: 'float' },
	            { name: 'ReleaseDate', type: 'datetime' }
	        ]
	    }]
	 });
	 // create a new time series stream aggregator which takes the values from the 'Price' field
	 // and the timestamps from the 'ReleaseDate' field. The window size should be 1 month.
	 var ts = {
	    name: 'GameSeries',
	    type: 'timeSeriesWinBuf',
	    store: 'GameCollection',
	    timestamp: 'ReleaseDate',
	    value: 'Price',
	    winsize: 2678400000
	 };
	 var timeSeries = base.store('GameCollection').addStreamAggr(ts);
	 // create a new sum stream aggregator
	 var sum = {
	    name: 'SumPrice',
	    type: 'winBufSum',
	    store: 'GameCollection',
	    inAggr: 'GameSeries'
	 };
	 var priceSum = base.store('GameCollection').addStreamAggr(sum);
	 // put some records in the store
	 base.store('GameCollection').push({ GameName: 'Tetris', Price: 0, ReleaseDate: '1984-06-06T00:00:00.0' });
	 base.store('GameCollection').push({ GameName: 'Super Mario Bros.', Price: 100, ReleaseDate: '1985-09-13T00:00:00.0' });
	 base.store('GameCollection').push({ GameName: 'The Legend of Zelda', Price: 90, ReleaseDate: '1986-02-21T00:00:00.0 '});
	 // get the timestamp of the last bought game by using getTimestamp
	 var date = priceSum.getTimestamp(); // returns 12153801600000 (the miliseconds since 1601-01-01T00:00:00.0)
	 base.close();
	
});
});
describe("Gets the length of the vector containing the values of the stream aggregator, number 21", function () {
it("should make test number 21", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a simple base containing one store
	 var base = new qm.Base({
	    mode: 'createClean',
	    schema: [{
	        name: 'IceCreams',
	        fields: [
	            { name: 'Type', type: 'string' },
	            { name: 'Price', type: 'float' },
	            { name: 'TimeOfConsumption', type: 'datetime' }
	        ]
	    }]
	 });
	 // create a time series stream aggregator, that takes values from the 'Price' field and the timestamp
	 //  from the 'TimeOfConsumation' field of 'IceCream' store. The window size should be 1 day.
	 var ts = {
	    name: 'IcePrice',
	    type: 'timeSeriesWinBuf',
	    store: 'IceCreams',
	    timestamp: 'TimeOfConsumption',
	    value: 'Price',
	    winsize: 86400000
	 };
	 var icePrice = base.store('IceCreams').addStreamAggr(ts);
	 // add some ice creams in the store
	 base.store('IceCreams').push({ Type: 'Chocholate', Price: 5, TimeOfConsumption: '2015-07-21T09:00:00.0' });
	 base.store('IceCreams').push({ Type: 'Blue Sky', Price: 3, TimeOfConsumption: '2015-07-21T14:13:00.0' });
	 base.store('IceCreams').push({ Type: 'Stracciatella', Price: 5, TimeOfConsumption: '2015-07-21T21:05:00.0' });
	 // get the number of ice creams consumed by using getFloatLength method
	 var numberOfIceCreamsEaten = icePrice.getFloatLength(); // returns 3
	 base.close();
	
});
});
describe("Returns the value of the vector containing the values of the stream aggregator at a specific index, number 22", function () {
it("should make test number 22", function () {
 
	 // import qm module
	 var qm = require('qminer');
	 // create a simple base containing one store
	 var base = new qm.Base({
	    mode: 'createClean',
	    schema: [{
	        name: 'MusicSale',
	        fields: [
	            { name: 'NumberOfAlbums', type: 'float' },
	            { name: 'Time', type: 'datetime' }
	        ]
	    }]
	 });
	 // create a time series containing the 'NumberOfAlbums' values and getting the timestamp from the 'Time' field.
	 // The window size should be 1 week.
	 var ts = {
	    name: 'Sales',
	    type: 'timeSeriesWinBuf',
	    store: 'MusicSale',
	    timestamp: 'Time',
	    value: 'NumberOfAlbums',
	    winsize: 604800000
	 };
	 var weekSales = base.store('MusicSale').addStreamAggr(ts);
	 // add some records in the store
	 base.store('MusicSale').push({ NumberOfAlbums: 10, Time: '2015-03-15T00:00:00.0' });
	 base.store('MusicSale').push({ NumberOfAlbums: 15, Time: '2015-03-18T00:00:00.0' });
	 base.store('MusicSale').push({ NumberOfAlbums: 30, Time: '2015-03-19T00:00:00.0' });
	 base.store('MusicSale').push({ NumberOfAlbums: 45, Time: '2015-03-20T00:00:00.0' });
	 // get the second value of the value vector 
	 var albums = weekSales.getFloatAt(1); // returns 15
	 base.close();
	
});
});
describe("Gets the whole vector of values of the stream aggregator, number 23", function () {
it("should make test number 23", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a simple base containing one store
	 var base = new qm.Base({
	    mode: 'createClean',
	    schema: [{
	        name: 'Hospital',
	        fields: [
	            { name: 'NumberOfPatients', type: 'float' },
	            { name: 'Date', type: 'datetime' }
	        ]
	    }]
	 });
	 // create a new time series stream aggregator that takes the values from the 'NumberOfPatients' field
	 // and the timestamp from the 'Date' field. The window size should be 1 week.
	 var ts = {
	    name: 'WeekPatients',
	    type: 'timeSeriesWinBuf',
	    store: 'Hospital',
	    timestamp: 'Date',
	    value: 'NumberOfPatients',
	    winsize: 604800000
	 };
	 var weekPatients = base.store('Hospital').addStreamAggr(ts);
	 // add some records in the store
	 base.store('Hospital').push({ NumberOfPatients: 50, Date: '2015-05-20T00:00:00.0' });
	 base.store('Hospital').push({ NumberOfPatients: 56, Date: '2015-05-21T00:00:00.0' });
	 base.store('Hospital').push({ NumberOfPatients: 120, Date: '2015-05-22T00:00:00.0' });
	 base.store('Hospital').push({ NumberOfPatients: 40, Date: '2015-05-23T00:00:00.0' });
	 // get the values that are in the time series window buffer as a vector
	 var values = weekPatients.getFloatVector(); // returns the vector [50, 56, 120, 40]
	 base.close();
	
});
});
describe("Gets the length of the timestamp vector of the stream aggregator, number 24", function () {
it("should make test number 24", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a simple base containing one store
	 var base = new qm.Base({
	    mode: 'createClean',
	    schema: [{
	        name: 'Medicine',
	        fields: [
	            { name: 'NumberOfPills', type: 'float' },
	            { name: 'Time', type: 'datetime' }
            ]
	    }]
	 });
	 // create a time series stream aggregator that takes the values from the 'NumberOfPills' field
	 // and the timestamp from the 'Time' field. The window size should be 1 week.
	 var ts = {
	    name: 'WeekPills',
	    type: 'timeSeriesWinBuf',
	    store: 'Medicine',
	    timestamp: 'Time',
	    value: 'NumberOfPills',
	    winsize: 604800000
	 };
	 var weekly = base.store('Medicine').addStreamAggr(ts);
	 // add some records in the store
	 base.store('Medicine').push({ NumberOfPills: 4, Time: '2015-07-21T09:00:00.0' });
	 base.store('Medicine').push({ NumberOfPills: 5, Time: '2015-07-21T19:00:00.0' });
	 base.store('Medicine').push({ NumberOfPills: 4, Time: '2015-07-22T09:00:00.0' });
	 base.store('Medicine').push({ NumberOfPills: 5, Time: '2015-07-22T19:00:00.0' });
	 base.store('Medicine').push({ NumberOfPills: 4, Time: '2015-07-23T09:00:00.0' });
	 base.store('Medicine').push({ NumberOfPills: 6, Time: '2015-07-23T19:00:00.0' });
	 base.store('Medicine').push({ NumberOfPills: 4, Time: '2015-07-24T09:00:00.0' });
	 // get the length of the timestamp vector
	 var length = weekly.getTimestampLength(); // returns 7
	 base.close();
	
});
});
describe("Gets the timestamp from the timestamp vector of the stream aggregator at the specific index, number 25", function () {
it("should make test number 25", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a simple base containing one store
	 var base = new qm.Base({
	    mode: 'createClean',
	    schema: [{
	        name: 'Route66',
	        fields: [
	            { name: 'NumberOfCars', type: 'float' },
	            { name: 'Time', type: 'datetime' }
	        ]
	    }]
	 });
	 // create a time series stream aggregator that takes the values from the 'NumberOfCars' field
	 // and the timestamps from the 'Time' field. The window size should be 1 day.
	 var ts = {
	    name: 'Traffic',
	    type: 'timeSeriesWinBuf',
	    store: 'Route66',
	    timestamp: 'Time',
	    value: 'NumberOfCars',
	    winsize: 86400000
	 };
	 var traffic = base.store('Route66').addStreamAggr(ts);
	 // add some records in the store
	 base.store('Route66').push({ NumberOfCars: 100, Time: '2015-06-15T06:00:00.0' });
	 base.store('Route66').push({ NumberOfCars: 88, Time: '2015-06-15T:10:00.0' });
	 base.store('Route66').push({ NumberOfCars: 60, Time: '2015-06-15T13:00:00.0' });
	 base.store('Route66').push({ NumberOfCars: 90, Time: '2015-06-15T18:00:00.0' });
	 base.store('Route66').push({ NumberOfCars: 110, Time: '2015-06-16T00:00:00.0' });
	 // get the third timestamp in the buffer
	 var time = traffic.getTimestampAt(2); // returns 13078864800000
	 base.close();
	
});
});
describe("Gets the vector containing the timestamps of the stream aggregator, number 26", function () {
it("should make test number 26", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a simple base containing one store
	 var base = new qm.Base({
	    mode: 'createClean',
	    schema: [{
	        name: 'Signals',
	        fields: [
	            { name: 'BeepLoudness', type: 'float' },
	            { name: 'Time', type: 'datetime' }
	        ]
	    }]
	 });
	 // create a time series stream aggregator that gets the values from the 'BeepLoudness' field and
	 // the timestamp from the 'Time' field. The window size should be 10 seconds.
	 var ts = {
	    name: 'SignalBeep',
	    type: 'timeSeriesWinBuf',
	    store: 'Signals',
	    timestamp: 'Time',
	    value: 'BeepLoudness',
	    winsize: 10000
	 };
	 var signalBeep = base.store('Signals').addStreamAggr(ts);
	 // add some records to the store
	 base.store('Signals').push({ BeepLoudness: 10, Time: '2015-07-21T12:30:30.0' });
	 base.store('Signals').push({ BeepLoudness: 25, Time: '2015-07-21T12:30:31.0' });
	 base.store('Signals').push({ BeepLoudness: 20, Time: '2015-07-21T12:30:32.0' });
	 // get the timestamp vector of signalBeep
	 var vec = signalBeep.getTimestampVector(); // returns vector [13081955430000, 13081955431000, 13081955432000]
	 base.close();
	
});
});
describe("Gets the value of the newest record added to the stream aggregator, number 27", function () {
it("should make test number 27", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a simple base containing one store
	 var base = new qm.Base({
	    mode: 'createClean',
	    schema: [{
	        name: 'Marathon',
	        fields: [
	            { name: 'Runner', type: 'string' },
	            { name: 'Speed', type: 'float' },
	            { name: 'Time', type: 'datetime' }
	        ]
	    }]
	 });
	 // create a time series stream aggregator that gets the values from the 'Speed' field
	 // and the timestamp from the 'Time' field. The window size should be 10 minutes.
	 var ts = {
	    name: 'Sensor',
	    type: 'timeSeriesWinBuf',
	    store: 'Marathon',
	    timestamp: 'Time',
	    value: 'Speed',
	    winsize: 600000
	 };
	 var sensor = base.store('Marathon').addStreamAggr(ts);
	 // add some records to the store
	 base.store('Marathon').push({ Runner: 'Marko Primozic', Speed: 13.4, Time: '2015-07-21T20:23:13.0' });
	 base.store('Marathon').push({ Runner: 'Leonard Cohen', Speed: 14.1, Time: '2015-07-21T20:24:01.0' });
	 base.store('Marathon').push({ Runner: 'Coco Chanelle', Speed: 13.7, Time: '2015-07-21T20:24:27.0' });
	 // get the last value that got in the buffer
	 var last = sensor.getInFloat(); // returns 13.7
	 base.close();
	
});
});
describe("Gets the timestamp of the newest record added to the stream aggregator, number 28", function () {
it("should make test number 28", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a simple base containing one store
	 var base = new qm.Base({
	    mode: 'createClean',
	    schema: [{
	        name: 'F1',
	        fields: [
	            { name: 'Driver', type: 'string' },
	            { name: 'Speed', type: 'float' },
	            { name: 'Time', type: 'datetime' }
	        ]
	    }]
	 });
	 // create a time series stream aggregator that gets the values from the 'Speed' field
	 // and the timestamp from the 'Time' field. The window size should be 5 minutes.
	 var ts = {
	    name: 'Sensor',
	    type: 'timeSeriesWinBuf',
	    store: 'F1',
	    timestamp: 'Time',
	    value: 'Speed',
	    winsize: 300000
	 };
	 var sensor = base.store('F1').addStreamAggr(ts);
	 // add some records to the store
	 base.store('F1').push({ Driver: 'Sebastian Vettel', Speed: 203.4, Time: '2015-07-19T09:32:01.0' });
	 base.store('F1').push({ Driver: 'Thomas "Tommy" Angelo', Speed: 152.8, Time: '2015-07-19T09:35:23.0' });
	 base.store('F1').push({ Driver: 'Mark Ham', Speed: 189.5, Time: '2015-07-19T09:38:43.0' });
	 // get the last timestamp that was added in the window buffer
	 var time = sensor.getInTimestamp(); // returns 13081772323000
	 base.close();
	
});
});
describe("Gets a vector containing the values that are leaving the stream aggregator, number 29", function () {
it("should make test number 29", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a simple base containing one store
	 var base = new qm.Base({
	    mode: 'createClean',
	    schema: [{
	        name: 'F1',
	        fields: [
	            { name: 'Driver', type: 'string' },
	            { name: 'Speed', type: 'float' },
	            { name: 'Time', type: 'datetime' }
	        ]
	    }]
	 });
	 // create a time series stream aggregator that gets the values from the 'Speed' field
	 // and the timestamp from the 'Time' field. The window size should be 5 minutes.
	 var ts = {
	    name: 'Sensor',
	    type: 'timeSeriesWinBuf',
	    store: 'F1',
	    timestamp: 'Time',
	    value: 'Speed',
	    winsize: 300000
	 };
	 var sensor = base.store('F1').addStreamAggr(ts);
	 // add some records to the store
	 base.store('F1').push({ Driver: 'Sebastian Vettel', Speed: 203.4, Time: '2015-07-19T09:32:01.0' });
	 base.store('F1').push({ Driver: 'Thomas "Tommy" Angelo', Speed: 152.8, Time: '2015-07-19T09:35:23.0' });
	 base.store('F1').push({ Driver: 'Mark Ham', Speed: 189.5, Time: '2015-07-19T09:38:43.0' });
	 base.store('F1').push({ Driver: 'Speedy Gonzales', Speed: 171.4, Time: '2015-07-19T09:40:32.0' });
	 // get the values, that have got out of the window buffer.
	 // because the window size is 5 seconds, the last value that have left the buffer is 152.8
	 var left = sensor.getOutFloatVector(); // returns [152.8]
	 base.close();
	
});
});
describe("Gets a vector containing the timestamps that are leaving the stream aggregator, number 30", function () {
it("should make test number 30", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a simple base containing one store
	 var base = new qm.Base({
	    mode: 'createClean',
	    schema: [{
	        name: 'Noise',
	        fields: [
	            { name: 'Decibels', type: 'float' },
	            { name: 'Time', type: 'datetime' }
	        ]
	    }]
	 });
	 // create a time series stream aggregator that takes the values from the 'Decibels' field
	 // and timestamps from the 'Time' fields. The window size should be 1 second.
	 var ts = {
	    name: 'Music',
	    type: 'timeSeriesWinBuf',
	    store: 'Noise',
	    timestamp: 'Time',
	    value: 'Decibels',
	    winsize: 1000
	 };
	 var music = base.store('Noise').addStreamAggr(ts);
	 // add some records in the store
	 base.store('Noise').push({ Decibels: 54, Time: '2015-07-21T14:43:00.0' });
	 base.store('Noise').push({ Decibels: 55, Time: '2015-07-21T14:43:00.200' });
	 base.store('Noise').push({ Decibels: 54, Time: '2015-07-21T14:43:00.400' });
	 base.store('Noise').push({ Decibels: 53, Time: '2015-07-21T14:43:00.600' });
	 base.store('Noise').push({ Decibels: 53, Time: '2015-07-21T14:43:00.800' });
	 base.store('Noise').push({ Decibels: 54, Time: '2015-07-21T14:43:01.0' });
	 base.store('Noise').push({ Decibels: 53, Time: '2015-07-21T14:43:01.2' });
	 // get the timestamps that just left the window buffer by adding the last record
	 var last = music.getOutTimestampVector(); // returns [13081963380000]
	 base.close();
	
});
});
describe("Gets the number of records in the stream aggregator, number 31", function () {
it("should make test number 31", function () {

	 // import qm module
	 var qm = require('qminer');
	 // create a simple base containing one store
	 var base = new qm.Base({
	    mode: 'createClean',
	    schema: [{
	        name: 'MusicSale',
	        fields: [
	            { name: 'NumberOfAlbums', type: 'float' },
	            { name: 'Time', type: 'datetime' }
	        ]
	    }]
	 });
	 // create a time series containing the values from the 'NumberOfAlbums' field and 
	 // the timestamp from the 'Time' field. The window size should be 1 week.
	 var ts = {
	    name: 'Sales',
	    type: 'timeSeriesWinBuf',
	    store: 'MusicSale',
	    timestamp: 'Time',
	    value: 'NumberOfAlbums',
	    winsize: 604800000
	 };
	 var weekSales = base.store('MusicSale').addStreamAggr(ts);
	 // add some records in the store
	 base.store('MusicSale').push({ NumberOfAlbums: 10, Time: '2015-03-15T00:00:00.0' });
	 base.store('MusicSale').push({ NumberOfAlbums: 15, Time: '2015-03-18T00:00:00.0' });
	 base.store('MusicSale').push({ NumberOfAlbums: 30, Time: '2015-03-19T00:00:00.0' });
	 base.store('MusicSale').push({ NumberOfAlbums: 45, Time: '2015-03-20T00:00:00.0' });
	 // get the number of records in the window buffer
	 var num = weekSales.getNumberOfRecords(); // returns 4
	 base.close();
	
});
});

});

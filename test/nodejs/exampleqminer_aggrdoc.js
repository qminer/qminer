describe('example tests for the qminer_aggrdoc.js file', function () {
describe("Qminer module., number 1", function () {
it("should make test number 1", function () {
 this.timeout(10000); 

 // import module
 var qm = require('qminer');

});
});
describe("Stream Aggregate, number 2", function () {
it("should make test number 2", function () {
 this.timeout(10000); 

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
describe("@typedef {module:qm.StreamAggr} StreamAggregateTimeSeriesWindow, number 3", function () {
it("should make test number 3", function () {
 this.timeout(10000); 
 
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
describe("@typedef {module:qm.StreamAggr} StreamAggregateSum, number 4", function () {
it("should make test number 4", function () {
 this.timeout(10000); 

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
describe("@typedef {module:qm.StreamAggr} StreamAggregateMin, number 5", function () {
it("should make test number 5", function () {
 this.timeout(10000); 

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
describe("@typedef {module:qm.StreamAggr} StreamAggregateMax, number 6", function () {
it("should make test number 6", function () {
 this.timeout(10000); 

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
describe("@typedef {module:qm.StreamAggr} StreamAggregateTimeSeriesTick, number 7", function () {
it("should make test number 7", function () {
 this.timeout(10000); 

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
describe("@typedef {module:qmStreamAggr} StreamAggregateMovingAverage, number 8", function () {
it("should make test number 8", function () {
 this.timeout(10000); 

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
describe("@typedef {module:qmStreamAggr} StreamAggregateEMA, number 9", function () {
it("should make test number 9", function () {
 this.timeout(10000); 

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
describe("@typedef {module:qm.StreamAggr} StreamAggregateMovingVariance, number 10", function () {
it("should make test number 10", function () {
 this.timeout(10000); 

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
describe("@typedef {module:qm.StreamAggr} StreamAggregateMovingCovariance, number 11", function () {
it("should make test number 11", function () {
 this.timeout(10000); 

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
describe("@typedef {module:qm.StreamAggr} StreamAggregateMovingCorrelation, number 12", function () {
it("should make test number 12", function () {
 this.timeout(10000); 

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
describe("@typedef {module:qm.StreamAggr} StreamAggregateResampler, number 13", function () {
it("should make test number 13", function () {
 this.timeout(10000); 

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
describe("@typedef {module:qm.StreamAggr} StreamAggregateMerger, number 14", function () {
it("should make test number 14", function () {
 this.timeout(10000); 

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
describe("@typedef {module:qm.StreamAggr} StreamAggregateHistogram, number 15", function () {
it("should make test number 15", function () {
 this.timeout(10000); 

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
describe("Returns the value of the specific stream aggregator. For return values see {@link module:qm~StreamAggregators}., number 16", function () {
it("should make test number 16", function () {
 this.timeout(10000); 

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
describe("Returns the timestamp value of the newest record in buffer., number 17", function () {
it("should make test number 17", function () {
 this.timeout(10000); 

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
describe("Gets the length of the vector containing the values of the stream aggregator., number 18", function () {
it("should make test number 18", function () {
 this.timeout(10000); 

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
describe("Returns the value of the vector containing the values of the stream aggregator at a specific index., number 19", function () {
it("should make test number 19", function () {
 this.timeout(10000); 
 
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
describe("Gets the whole vector of values of the stream aggregator., number 20", function () {
it("should make test number 20", function () {
 this.timeout(10000); 

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
describe("Gets the length of the timestamp vector of the stream aggregator., number 21", function () {
it("should make test number 21", function () {
 this.timeout(10000); 

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
describe("Gets the timestamp from the timestamp vector of the stream aggregator at the specific index., number 22", function () {
it("should make test number 22", function () {
 this.timeout(10000); 

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
describe("Gets the vector containing the timestamps of the stream aggregator., number 23", function () {
it("should make test number 23", function () {
 this.timeout(10000); 

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
describe("Gets the value of the newest record added to the stream aggregator., number 24", function () {
it("should make test number 24", function () {
 this.timeout(10000); 

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
describe("Gets the timestamp of the newest record added to the stream aggregator., number 25", function () {
it("should make test number 25", function () {
 this.timeout(10000); 

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
describe("Gets a vector containing the values that are leaving the stream aggregator., number 26", function () {
it("should make test number 26", function () {
 this.timeout(10000); 

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
describe("Gets a vector containing the timestamps that are leaving the stream aggregator., number 27", function () {
it("should make test number 27", function () {
 this.timeout(10000); 

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
describe("Gets the number of records in the stream aggregator., number 28", function () {
it("should make test number 28", function () {
 this.timeout(10000); 

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

describe("@typedef {module:qm.StreamAggr} StreamAggregateChiSquare, number 29", function () {
it("should make test number 29", function () {
	this.timeout(10000); 

	// import the qm module
	var qm = require('qminer');
 
	var store = undefined

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

	// add ChiSquare aggregator that connects with Histogram1 with expected values and Histogram2 with actual values
	aggr = {
		name: 'ChiAggr',
		type: 'chiSquare',
		storeX: 'Dice',
		storeY: 'Dice',
		inAggrX: 'Histogram1',
		inAggrY: 'Histogram2',
		degreesOfFreedom: 2
	};

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

	base.close();

	});
});
});

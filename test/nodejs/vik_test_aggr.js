var assert = require("assert");

describe("Viktor test aggregates", function () {
	it("window+histogram", function () {
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
			winsize: 2 * 60 * 60 * 1000 // 2 hours
		};

		var timeSeries1 = base.store("Dice").addStreamAggr(timeser);

		// add a histogram aggregator, that is connected with the 'TimeSeries1' aggregator
		var aggrJson = {
			name: 'Histogram1',
			type: 'onlineHistogram',
			store: 'Dice',
			inAggr: 'TimeSeries1',
			lowerBound: 0,
			upperBound: 6,
			bins: 6,
			addNegInf: false,
			addPosInf: false
		};

		var hist1 = base.store("Dice").addStreamAggr(aggrJson);

		// add some values
		// simulating throwing a dice
		store.push({ Time: '2015-06-10T14:13:30.0', Expected: 0, Observed: 1 });
		store.push({ Time: '2015-06-10T14:13:31.0', Expected: 0, Observed: 2 });
		store.push({ Time: '2015-06-10T14:13:32.0', Expected: 1, Observed: 3 });
		store.push({ Time: '2015-06-10T14:13:33.0', Expected: 1, Observed: 4 });
		store.push({ Time: '2015-06-10T14:13:34.0', Expected: 2, Observed: 5 });
		store.push({ Time: '2015-06-10T14:13:35.0', Expected: 3, Observed: 5 });

		var tmp1 = hist1.saveJson();
		console.log(JSON.stringify(tmp1));
		
		store.push({ Time: '2015-06-10T14:13:41.0', Expected: 3, Observed: 5 });
		store.push({ Time: '2015-06-10T14:13:41.0', Expected: 3, Observed: 5 });
		store.push({ Time: '2015-06-10T14:13:42.0', Expected: 4, Observed: 5 });
		store.push({ Time: '2015-06-10T14:13:43.0', Expected: 4, Observed: 5 });
		store.push({ Time: '2015-06-10T14:13:44.0', Expected: 5, Observed: 6 });
		store.push({ Time: '2015-06-10T14:13:45.0', Expected: 1, Observed: 6 });

		var tmp2 = hist1.saveJson();
		console.log(JSON.stringify(tmp2));

		store.push({ Time: '2015-06-10T17:13:50.0', Expected: 1, Observed: 6 });
		store.push({ Time: '2015-06-10T17:13:51.0', Expected: 2, Observed: 6 });
		store.push({ Time: '2015-06-10T17:13:52.0', Expected: 3, Observed: 6 });
		store.push({ Time: '2015-06-10T17:13:53.0', Expected: 4, Observed: 6 });
		store.push({ Time: '2015-06-10T17:13:54.0', Expected: 5, Observed: 6 });
		store.push({ Time: '2015-06-10T17:13:55.0', Expected: 6, Observed: 6 });

		var tmp3 = hist1.saveJson();
		console.log(JSON.stringify(tmp3));

		// show distribution for expected values
		console.log(hist1);

		base.close();
	});
	it("slotted histogram", function () {
		this.timeout(10000); 

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
		var timeser = {
			name: 'TimeSeries1',
			type: 'timeSeriesWinBuf',
			store: 'Rpm',
			timestamp: 'Time',
			value: 'ClusterId',
			winsize: 4 * 7 * 24 * 60 * 60 * 1000 // 4 weeks
		};

		var timeSeries1 = base.store("Rpm").addStreamAggr(timeser);

		// add a slotted-histogram aggregator that is connected with the 'TimeSeries1' aggregator
		var aggrJson = {
			name: 'Histogram1',
			type: 'onlineSlottedHistogram',
			store: 'Rpm',
			inAggr: 'TimeSeries1',
			period: 7 * 24 * 60 * 60 * 1000, // 1 day
			window: 2 * 60 * 60 * 1000, // 2h
			bins: 5, // 5 possible clusters
			granularity: 5 * 60 * 1000  // 5 min
		};

		var hist1 = base.store("Rpm").addStreamAggr(aggrJson);

		// add some values
		store.push({ Time: '2015-06-10T00:13:30.0', ClusterId: 0 });
		store.push({ Time: '2015-06-10T00:14:30.0', ClusterId: 0 });
		store.push({ Time: '2015-06-10T00:15:31.0', ClusterId: 1 });
		store.push({ Time: '2015-06-10T00:16:30.0', ClusterId: 0 });
		store.push({ Time: '2015-06-10T00:17:30.0', ClusterId: 0 });
		store.push({ Time: '2015-06-10T00:18:30.0', ClusterId: 0 });

		var tmp1 = hist1.saveJson();
		assert.equal(tmp1.counts.length, 5);
		assert.equal(tmp1.counts[0], 5);
		assert.equal(tmp1.counts[1], 1);
		assert.equal(tmp1.counts[2], 0);
		assert.equal(tmp1.counts[3], 0);
		assert.equal(tmp1.counts[4], 0);
		//console.log(JSON.stringify(tmp1));
		
		store.push({ Time: '2015-06-10T01:13:30.0', ClusterId: 0 });
		store.push({ Time: '2015-06-10T02:14:30.0', ClusterId: 0 });
		store.push({ Time: '2015-06-10T03:15:31.0', ClusterId: 1 });
		store.push({ Time: '2015-06-10T04:16:30.0', ClusterId: 0 });
		store.push({ Time: '2015-06-10T05:17:30.0', ClusterId: 2 });
		store.push({ Time: '2015-06-10T06:18:30.0', ClusterId: 2 });

		// just the last three
		var tmp2 = hist1.saveJson();
		assert.equal(tmp2.counts[0], 1);
		assert.equal(tmp2.counts[1], 0);
		assert.equal(tmp2.counts[2], 2);
		assert.equal(tmp2.counts[3], 0);
		assert.equal(tmp2.counts[4], 0);
		//console.log(JSON.stringify(tmp2));

		store.push({ Time: '2015-06-17T00:13:30.0', ClusterId: 2 });
		store.push({ Time: '2015-06-17T00:14:30.0', ClusterId: 2 });
		store.push({ Time: '2015-06-17T00:15:31.0', ClusterId: 3 });
		store.push({ Time: '2015-06-17T00:16:30.0', ClusterId: 3 });
		store.push({ Time: '2015-06-17T00:17:30.0', ClusterId: 4 });
		store.push({ Time: '2015-06-17T00:18:30.0', ClusterId: 4 });

		// this and previous week¸
		var tmp3 = hist1.saveJson();
		assert.equal(tmp3.counts[0], 5);
		assert.equal(tmp3.counts[1], 1);
		assert.equal(tmp3.counts[2], 2);
		assert.equal(tmp3.counts[3], 2);
		assert.equal(tmp3.counts[4], 2);
		//console.log(JSON.stringify(tmp3));

		// show distribution for expected values
		//console.log(hist1);

		base.close();
	});
});
var assert = require("assert");

describe("test histogram, slotted-histogram and histogram_diff aggregates", function () {
	it("histogram", function () {
		this.timeout(10000); 
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
		try {
			var store = base.store('Rpm');

			// create a new time series stream aggregator for the 'Rpm' store that takes the recorded cluster id
			// and the timestamp from the 'Time' field. The size of the window is 2 hours.
			var timeser = {
				name: 'TimeSeries1',
				type: 'timeSeriesWinBuf',
				store: 'Rpm',
				timestamp: 'Time',
				value: 'ClusterId',
				winsize: 2 * 60 * 60 * 1000 // 2 hours
			};
			var timeSeries1 = base.store("Rpm").addStreamAggr(timeser);

			// add a histogram aggregator, that is connected with the 'TimeSeries1' aggregator
			var aggrJson = {
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
			store.push({ Time: '2015-06-10T04:16:30.0', ClusterId: 0 }); // this one will just leave the window
			store.push({ Time: '2015-06-10T05:17:30.0', ClusterId: 2 });
			store.push({ Time: '2015-06-10T06:18:30.0', ClusterId: 2 });

			// just the last three
			var tmp2 = hist1.saveJson();
			assert.equal(tmp2.counts[0], 0);
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

			// this week
			var tmp3 = hist1.saveJson();
			assert.equal(tmp3.counts[0], 0);
			assert.equal(tmp3.counts[1], 0);
			assert.equal(tmp3.counts[2], 2);
			assert.equal(tmp3.counts[3], 2);
			assert.equal(tmp3.counts[4], 2);
			//console.log(JSON.stringify(tmp3));

			// show distribution for expected values
			//console.log(hist1);
		} finally {
			base.close();
		}
	});
	it("slotted histogram", function () {
		this.timeout(10000); 
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
		try {
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
				period: 7 * 24 * 60 * 60 * 1000, // 1 week
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

			// this and previous week
			var tmp3 = hist1.saveJson();
			assert.equal(tmp3.counts[0], 5);
			assert.equal(tmp3.counts[1], 1);
			assert.equal(tmp3.counts[2], 2);
			assert.equal(tmp3.counts[3], 2);
			assert.equal(tmp3.counts[4], 2);
			//console.log(JSON.stringify(tmp3));

			// show distribution for expected values
			//console.log(hist1);

		} finally {
			base.close();
		}
	});
	it("histogram_diff simple", function () {
		this.timeout(10000); 
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
		try {
			var store = base.store('Rpm');

			// create a new time series stream aggregator for the 'Rpm' store that takes the recorded cluster id
			// and the timestamp from the 'Time' field. The size of the window is 4 weeks.
			var timeser1 = {
				name: 'TimeSeries1',
				type: 'timeSeriesWinBuf',
				store: 'Rpm',
				timestamp: 'Time',
				value: 'ClusterId',
				winsize: 2 * 60 * 60 * 1000 // 2 hours
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
				winsize: 6 * 60 * 60 * 1000 // 6 hours
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

			////////////////////////////////////////////////////////////////////
				
			// add some values
			store.push({ Time: '2015-06-10T00:13:30.0', ClusterId: 0 });
			store.push({ Time: '2015-06-10T00:14:30.0', ClusterId: 0 });
			store.push({ Time: '2015-06-10T01:15:31.0', ClusterId: 1 });
			store.push({ Time: '2015-06-10T02:16:30.0', ClusterId: 2 });
			store.push({ Time: '2015-06-10T03:17:30.0', ClusterId: 0 });
			store.push({ Time: '2015-06-10T04:18:30.0', ClusterId: 2 });

			var tmp1 = hist1.saveJson();
			//console.log(JSON.stringify(tmp1));
			assert.equal(tmp1.counts.length, 5);
			assert.equal(tmp1.counts[0], 1);
			assert.equal(tmp1.counts[1], 0);
			assert.equal(tmp1.counts[2], 1);
			assert.equal(tmp1.counts[3], 0);
			assert.equal(tmp1.counts[4], 0);

			// just the last three
			var tmp2 = hist2.saveJson();
			//console.log(JSON.stringify(tmp2));
			assert.equal(tmp2.counts[0], 3);
			assert.equal(tmp2.counts[1], 1);
			assert.equal(tmp2.counts[2], 2);
			assert.equal(tmp2.counts[3], 0);
			assert.equal(tmp2.counts[4], 0);
						
			// difference
			var tmp3 = diff.saveJson();
			//console.log(JSON.stringify(tmp3));
			assert.equal(tmp3.diff[0], 2);
			assert.equal(tmp3.diff[1], 1);
			assert.equal(tmp3.diff[2], 1);
			assert.equal(tmp3.diff[3], 0);
			assert.equal(tmp3.diff[4], 0);
			
			// show distribution for expected values
			//console.log(diff);
			
		} finally {
			base.close();
		}
		
		var base2 = new qm.Base({
			mode: "open"
		});		
		try {
			var store2 = base2.store('Rpm');
        
			// get diff aggregator that subtracts Histogram1 with 2h window from Histogram2 with 6h window
			var diff = store2.getStreamAggr("DiffAggr");
						
			// difference
			var tmp3 = diff.saveJson();
			console.log(JSON.stringify(tmp3));
			assert.equal(tmp3.diff[0], 2);
			assert.equal(tmp3.diff[1], 1);
			assert.equal(tmp3.diff[2], 1);
			assert.equal(tmp3.diff[3], 0);
			assert.equal(tmp3.diff[4], 0);
			
		} finally {
			base2.close();
		}
	});
});
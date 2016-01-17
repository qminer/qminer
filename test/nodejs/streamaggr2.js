var assert = require("assert");

describe("test histogram, slotted-histogram and histogram_diff aggregates", function () {
	it("histogram", function () {
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
	it("histogram - sfloat", function () {
		var qm = require('qminer');

		// create a base with a simple store
		// the store records results of clustering
		var base = new qm.Base({
			mode: "createClean",
			schema: [
			{
				name: "Rpm",
				fields: [
					{ name: "ClusterId", type: "sfloat" },
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
	it("histogram reset", function () {
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

            // RESET
	        store.resetStreamAggregates();

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


	        // show distribution for expected values
	        //console.log(hist1);
	    } finally {
	        base.close();
	    }
	});
	it("slotted histogram", function () {
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
	it("slotted histogram reset", function () {
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

	        store.resetStreamAggregates();

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


	        // show distribution for expected values
	        //console.log(hist1);

	    } finally {
	        base.close();
	    }
	});
	it("histogram_diff simple", function () {
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
	});
	it("histogram_diff - complex - reset and reload", function () {
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

	        store.resetStreamAggregates();

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
			
			/////////////////////////////////////////////////////////
			var fout = qm.fs.openWrite("aggr.tmp");
			hist1.save(fout);			
			timeSeries1.save(fout);
			hist2.save(fout);			
			timeSeries2.save(fout);
			diff.save(fout);			
			fout.close();

			store.resetStreamAggregates();
				
			var fin = qm.fs.openRead("aggr.tmp");
			hist1.load(fin);
			timeSeries1.load(fin);
			hist2.load(fin);
			timeSeries2.load(fin);
			diff.load(fin);
			fin.close();

			var tmp1x = hist1.saveJson();
	        var tmp2x = hist2.saveJson();
	        var tmp3x = diff.saveJson();
			
			assert.equal(JSON.stringify(tmp1), JSON.stringify(tmp1x));
			assert.equal(JSON.stringify(tmp2), JSON.stringify(tmp2x));
			assert.equal(JSON.stringify(tmp3), JSON.stringify(tmp3x));
	    } finally {
	        base.close();
	    }
	});

});



describe('Time Series Window Buffer Feature Extractor', function () {
    var qm = require('qminer');
    var base = undefined;
    var store = undefined;
    beforeEach(function () {
        base = new qm.Base({
            mode: 'createClean',
            schema: [{
                name: 'Docs',
                fields: [
                    { name: 'Time', type: 'datetime' },
                    { name: 'Text', type: 'string' }
                ]
            }]
        });
        store = base.store('Docs');
    });
    afterEach(function () {
        base.close();
    });

    describe('Constructor Tests', function () {
        it('should construct the time series window buffer', function () {
            var aggr = {
                name: 'featureSpaceWindow',
                type: 'timeSeriesWinBufFeatureSpace',
                store: 'Docs',
                timestamp: 'Time',
                featureSpace: {
                    type: "categorical",
                    source: "Docs",
                    field: "Text"
                },
                winsize: 1000
            };
            var sa = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Text: 'a' }); // 0
            store.push({ Time: '2015-06-10T14:13:33.0', Text: 'b' }); // 1
            store.push({ Time: '2015-06-10T14:14:34.0', Text: 'c' }); // 2
            store.push({ Time: '2015-06-10T14:15:35.0', Text: 'd' }); // 3
            store.push({ Time: '2015-06-10T14:15:36.0', Text: 'e' }); // 4
			store.push({ Time: '2015-06-10T14:15:37.0', Text: 'f' }); // 5
			// 6 dim,  vals indices {4,5}, in {5}, out {3}
            var inValVec = sa.getInValueVector();
            assert.equal(inValVec.cols, 1);
            assert.equal(inValVec.full().minus(new qm.la.Matrix([[0], [0], [0], [0], [0], [1]])).frob(), 0);
            var outValVec = sa.getOutValueVector();
            assert.equal(outValVec.cols, 1);
            assert.equal(outValVec.full().minus(new qm.la.Matrix([[0], [0], [0], [1]])).frob(), 0);
            var valVec = sa.getValueVector();
            assert.equal(valVec.cols, 2);
            assert.equal(valVec.full().minus(new qm.la.Matrix([[0,0], [0,0], [0,0], [0,0], [1,0], [0,1]])).frob(), 0);
        });
    });
    
    describe('Feature space getter', function () {
        it('should return the internal feature space', function () {
            var aggr = {
                name: 'featureSpaceWindow',
                type: 'timeSeriesWinBufFeatureSpace',
                store: 'Docs',
                timestamp: 'Time',
                featureSpace: {
                    type: "categorical",
                    source: "Docs",
                    field: "Text"
                },
                winsize: 1000
            };
            var sa = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Text: 'a' }); // 0
            store.push({ Time: '2015-06-10T14:13:33.0', Text: 'b' }); // 1
            store.push({ Time: '2015-06-10T14:14:34.0', Text: 'c' }); // 2
            store.push({ Time: '2015-06-10T14:15:35.0', Text: 'd' }); // 3
            store.push({ Time: '2015-06-10T14:15:36.0', Text: 'e' }); // 4
			store.push({ Time: '2015-06-10T14:15:37.0', Text: 'f' }); // 5				
		    var featureSpace = sa.getFeatureSpace();
		    assert.equal(featureSpace.dim, 6);
        });
    });

	describe('WinBuf Feature serialzie/desiralize', function () {
        it('should save and load the stream aggregate', function () {
            var aggr = {
                name: 'featureSpaceWindow',
                type: 'timeSeriesWinBufFeatureSpace',
                store: 'Docs',
                timestamp: 'Time',
                featureSpace: {
                    type: "categorical",
                    source: "Docs",
                    field: "Text"
                },
                winsize: 1000
            };
            var sa = store.addStreamAggr(aggr);
            store.push({ Time: '2015-06-10T14:13:32.0', Text: 'a' }); // 0
            store.push({ Time: '2015-06-10T14:13:33.0', Text: 'b' }); // 1
            store.push({ Time: '2015-06-10T14:14:34.0', Text: 'c' }); // 2
            store.push({ Time: '2015-06-10T14:15:35.0', Text: 'd' }); // 3
            store.push({ Time: '2015-06-10T14:15:36.0', Text: 'e' }); // 4
			store.push({ Time: '2015-06-10T14:15:37.0', Text: 'f' }); // 5
			
			var fout = qm.fs.openWrite('fsWinBuf.bin');
			sa.save(fout).close();
			var fin = qm.fs.openRead('fsWinBuf.bin');
            sa.load(fin);
		    var featureSpace = sa.getFeatureSpace();
		    assert.equal(featureSpace.dim, 6);
        });
    });

    describe('Complex Tests', function () {
        it('should construct the time-series window-buffer and attach sum to it', function () {
            var aggr = {
                name: 'featureSpaceWindow',
                type: 'timeSeriesWinBufFeatureSpace',
                store: 'Docs',
                timestamp: 'Time',
                featureSpace: {
                    type: "categorical",
                    source: "Docs",
                    field: "Text"
                },
                winsize: 1000
            };
            var sa = store.addStreamAggr(aggr);
            var aggr2 = {
                name: 'sparseVectorSum',
                type: 'winBufSpVecSum',
                store: 'Docs',
                inAggr: 'featureSpaceWindow'
            };
            var sa2 = store.addStreamAggr(aggr2);
            store.push({ Time: '2015-06-10T14:13:32.0', Text: 'a' }); // 0
            store.push({ Time: '2015-06-10T14:13:33.0', Text: 'b' }); // 1
            store.push({ Time: '2015-06-10T14:14:34.0', Text: 'c' }); // 2
            store.push({ Time: '2015-06-10T14:15:35.0', Text: 'd' }); // 3
            store.push({ Time: '2015-06-10T14:15:36.0', Text: 'e' }); // 4
            store.push({ Time: '2015-06-10T14:15:37.0', Text: 'f' }); // 5
            
			// 6 dim,  vals indices {4,5}, in {5}, out {3}
            var inValVec = sa.getInValueVector();
            assert.equal(inValVec.cols, 1);
            assert.equal(inValVec.full().minus(new qm.la.Matrix([[0], [0], [0], [0], [0], [1]])).frob(), 0);
            var outValVec = sa.getOutValueVector();
            assert.equal(outValVec.cols, 1);
            assert.equal(outValVec.full().minus(new qm.la.Matrix([[0], [0], [0], [1]])).frob(), 0);
            var valVec = sa.getValueVector();
            assert.equal(valVec.cols, 2);
            assert.equal(valVec.full().minus(new qm.la.Matrix([[0,0], [0,0], [0,0], [0,0], [1,0], [0,1]])).frob(), 0);
			
			var valVec2 = sa2.getValueVector();
            assert.equal(valVec2.full().minus(new qm.la.Vector([0, 0, 0, 0, 1, 1])).norm(), 0);
        });
    });
});



describe('Time Series  - EMA for sparse vectors', function () {
    var qm = require('qminer');
    var base = undefined;
    var store = undefined;
    beforeEach(function () {
        base = new qm.Base({
            mode: 'createClean',
            schema: [{
                name: 'Docs',
                fields: [
                    { name: 'Time', type: 'datetime' },
                    { name: 'Text', type: 'string' }
                ]
            }]
        });
        store = base.store('Docs');
    });
    afterEach(function () {
        base.close();
    });
    
    it('should construct sparse-vector time-series - EMA', function () {
        var aggr = {
            name: 'featureSpaceWindow',
            type: 'timeSeriesWinBufFeatureSpace',
            store: 'Docs',
            timestamp: 'Time',
            featureSpace: {
                type: "categorical",
                source: "Docs",
                field: "Text"
            },
            winsize: 0 // keep only most recent value in window
        };
        var sa = store.addStreamAggr(aggr);
        var aggr2 = {
            name: 'sparseVectorSum',
            type: 'winBufSpVecSum',
            store: 'Docs',
            inAggr: 'featureSpaceWindow' // this means that sum is equal to the most recent data
        };
        var sa2 = store.addStreamAggr(aggr2);
        var aggr3 = {
            name: 'sparseVectorEma',
            type: 'emaSpVec',
            store: 'Docs',
            inAggr: 'sparseVectorSum',
            emaType: "next",
            interval: 2000,
            initWindow: 0
        };
        var sa3 = store.addStreamAggr(aggr3);
        store.push({ Time: 1000, Text: 'a' }); // 0
        //sa3.getValueVector().print();
        store.push({ Time: 2000, Text: 'b' }); // 1
        //sa3.getValueVector().print();
        store.push({ Time: 3000, Text: 'c' }); // 2
        //sa3.getValueVector().print();
        store.push({ Time: 4000, Text: 'd' }); // 3
        //sa3.getValueVector().print();
        store.push({ Time: 5000, Text: 'e' }); // 4
        //sa3.getValueVector().print();
        store.push({ Time: 6000, Text: 'f' }); // 5
        //sa3.getValueVector().print();
                    
        var valVec2 = sa2.getValueVector();
        //valVec2.print();
        assert.equal(valVec2.full().minus(new qm.la.Vector([0, 0, 0, 0, 0, 1])).norm(), 0);

        var valVec3 = sa3.getValueVector();
        //valVec3.print();
        //console.log(valVec3.full().length);
        assert.ok(valVec3.full().minus(new qm.la.Vector([
            0.082085,
            0.05325, 
            0.087795,
            0.144749,
            0.238651,
            0.393469])).norm() < 0.000001);
    });    
});
// import analytics module
var analytics = require('analytics.js');
var assert = require('assert.js');

function testClassificationContAttr() {
	// algorithm parameters 
	var htParams = {
		"gracePeriod": 300,
		"splitConfidence": 1e-6,
		"tieBreaking": 0.01,
		"driftCheck": 1000,
		"windowSize": 100000,
		"conceptDriftP": true
	};
	
	// describe the data stream 
	var seaCfg = {
		"dataFormat": ["A1", "A2", "A3", "C"], 
		"A1": {
			"type": "numeric",
		}, 
		"A2": {
			"type": "numeric",
		}, 
		"A3": {
			"type": "numeric"
		}, 
		"C": {
			"type": "discrete",
			"values": ["0", "1"]
		}
	};

	// create a new learner 
	var ht = analytics.newHoeffdingTree(seaCfg, htParams);

	// train the model
	var streamData = fs.openRead("./sandbox/ht/sea.dat");
	var examplesN = 0;
	while (!streamData.eof) {
		var line = streamData.getNextLn().split(",");
		// get discrete attributes
		var example_discrete = [];
		// get numeric attributes
		var example_numeric = line.slice(0, 3);
		example_numeric = example_numeric.map(parseFloat);
		// get target
		var target = line[3];
		if (++examplesN % 10000 == 0) {
			console.say("Processing example number " + examplesN);
		}
		// update the model
		ht.process(example_discrete, example_numeric, target);
	}
	// Note that this is not how you'd test a streaming classifier in practice 
	// This is just playing around 
	streamData = fs.openRead("./sandbox/ht/sea.dat");
	examplesN = 0;
	var correct = 0;
	while (!streamData.eof) {
		var line = streamData.getNextLn().split(",");
		// get discrete attributes
		var example_discrete = [];
		// get numeric attributes
		var example_numeric = line.slice(0, 3);
		example_numeric = example_numeric.map(parseFloat);
		// get target
		var target = line[3];
		if (++examplesN % 10000 == 0) {
			console.say("Testing on example number " + examplesN);
		}
		// update the model
		if (parseFloat(ht.classify(example_discrete, example_numeric)) == target) {
			++correct;
		}
	}
	console.say("ACC = " + (correct/examplesN));
	// use the model 
	var label = ht.classify([], [6.677259,5.152133,2.982455]);
	console.say("f(6.677259,5.152133,2.982455) = " + label);
	var label = ht.classify([], [1.848014,0.041624,2.913719]);
	console.say("f(1.848014,0.041624,2.913719) = " + label);

	// export the model 
	ht.exportModel({ "file": "./sandbox/ht/sea.gv", "type": "DOT" });
}

function testClassification() {
	// algorithm parameters 
	var htParams = {
		"gracePeriod": 300,
		"splitConfidence": 1e-6,
		"tieBreaking": 0.01,
		"driftCheck": 1000,
		"windowSize": 100000,
		"conceptDriftP": false
	};
	
	// describe the data stream 
	var titanicCfg = {
		"dataFormat": ["status", "age", "sex", "survived"], 
		"sex": {
			"type": "discrete",
			"values": ["male", "female"]
		}, 
		"status": {
			"type": "discrete",
			"values": ["first", "second", "third", "crew"]
		}, 
		"age": {
			"type": "discrete",
			"values": ["child", "adult"]
		}, 
		"survived": {
			"type": "discrete",
			"values": ["yes", "no"]
		}
	};
	// create a new learner 
	var ht = analytics.newHoeffdingTree(titanicCfg, htParams);

	// train the model
	var streamData = fs.openRead("./sandbox/ht/titanic-4M.dat");
	var examplesN = 0;
	while (!streamData.eof) {
		var line = streamData.getNextLn().split(","); // male,first,adult,yes
		// get discrete attributes
		var example_discrete = line.slice(0, 3);
		// get numeric attributes
		var example_numeric = [];
		// get target
		var target = line[3];
		if (++examplesN % 10000 == 0) {
			console.say("Processing example number " + examplesN);
		}
		if (examplesN > 500000) { break; }
		// update the model
		ht.process(example_discrete, example_numeric, target);
	}
	
	// use the model 
	var label = ht.classify(["first", "adult", "female"], []);
	console.say("Were high society women likely to survive? " + label);
	var label = ht.classify(["first", "adult", "male"], []);
	console.say("Were high society men likely to survive? " + label);
	var label = ht.classify(["third", "adult", "male"], []);
	console.say("Were 3rd class men likely to survive? " + label);
	var label = ht.classify(["second", "adult", "female"], []);
	console.say("Were 3rd class women likely to survive? " + label);

	// export the model 
	ht.exportModel({ "file": "./sandbox/ht/titanic.gv", "type": "DOT" });
}

function testRegression() {
	// algorithm parameters 
	var htParams = {
		"gracePeriod": 300,
		"splitConfidence": 1e-6,
		"tieBreaking": 0.005,
		"driftCheck": 1000,
		"windowSize": 100000,
		"conceptDriftP": true
	};
	
	// describe the data stream 
	var regTestCfg = {
		"dataFormat": ["A", "B", "Y"],
		"A": {
			"type": "discrete",
			"values": ["t", "f"]
		},
		"B": {
			"type": "discrete",
			"values": ["t", "f"]
		},
		"Y": {
			"type": "numeric"
		}
	};
	
	// create a new learner 
	var ht = analytics.newHoeffdingTree(regTestCfg, htParams);

	// train the model
	var examplesN = 0;
	var streamData = fs.openRead("./sandbox/ht/regression-test.txt");
	while (!streamData.eof) {
		var line = streamData.getNextLn().split(",");
		// get discrete attributes
		var example_discrete = line.slice(0, 2);
		// get numeric attributes
		var example_numeric = [];
		// get target
		var target = parseFloat(line[2]);
		// entertain the user in case we have too many learning examples 
		if (++examplesN % 10000 == 0) {
			console.say("Number of examples processed so far: "+examplesN);
			if (examplesN > 200000) { break; }
		}
		// update the model
		ht.process(example_discrete, example_numeric, target);
	}
	
	// use the model 
	var val = ht.predict(["t", "t"], []);
	console.say("f(t,t) = " + val);
	var val = ht.predict(["t", "f"], []);
	console.say("f(t,f) = " + val);
	var val = ht.predict(["f", "t"], []);
	console.say("f(f,t) = " + val);
	var val = ht.predict(["f", "f"], []);
	console.say("f(f,f) = " + val);
	
	// export the model 
	ht.exportModel({ "file": "./sandbox/ht/regression-test.gv", "type": "DOT" });
}

console.say(" --- Example using classification HoeffdingTree --- ");
// testClassificationContAttr();
testClassification();
// console.say(" --- Example using regression HoeffdingTree --- ");
// testRegression();

console.say("Interactive mode: empty line to release (press ENTER).");
console.start();


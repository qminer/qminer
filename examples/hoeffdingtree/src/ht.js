console.say("HoeffdingTree test");
var analytics = require('analytics');
var assert = require('assert.js');

// describe the data stream 
var titanicConfig = {
	"dataFormat": ["status", "sex", "age", "survived"], 
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

// algorithm parameters 
var htParams = {
	"gracePeriod": 300,
	"splitConfidence": 1e-6,
	"tieBreaking": 0.01,
	"driftCheck": 1000,
	"windowSize": 100000,
	"conceptDriftP": false
};

// create a new learner 
var ht = analytics.newHoeffdingTree(titanicConfig, htParams);

// train the model
var streamData = fs.openRead("./sandbox/hoeffdingtree/titanic-220K.dat");
while(!streamData.eof) {
	var line = streamData.getNextLn().split(",");
	// get discrete attributes
	var example_discrete = line.slice(0,3);
	// get numeric attributes
	var example_numeric = [];
	// get target
	var target = line[3];	
	// update the model
	ht.process(example_discrete, example_numeric, target);
}

// use the model 
var label = ht.classify(["first", "adult", "female"], []);
console.say("Were high society women likely to survive? " + label);

// export the model 
ht.exportModel({ "file": "./sandbox/hoeffdingtree/titanic.gv", "type": "DOT" });

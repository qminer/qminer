// This example demonstrates text mining (feature vectors, active learning and classification)
// as well as record set filtering (based on time and classification results). It also builds 
// communication graphs based on sets of twitter messages (twitter specific)

// Import libraries
var analytics = require('analytics.js');
var assert = require('assert.js');
var time = require('time');

// Load tweets from a file (toy example)
// Set the filename
var tweetsFile = "./sandbox/twitter/toytweets.txt";
// Get the store
var Tweets = qm.store("Tweets");
// Load tweets (each line is a json)
qm.load.jsonFile(Tweets, tweetsFile);
// Print number of records
console.say("number of records: " + Tweets.length);
// Select all tweets
var recSet = Tweets.recs;

// Active learning settings: start svm when 2 positive and 2 negative examples are provided
var nPos = 2; var nNeg = 2; //active learning query mode
// Initial query for "relevant" documents
var relevantQuery = "nice bad";
// Initial query for positive sentiment
var sentimentQuery = "nice";

// Compute the feature space (if buildFtrSpace is false loads it from disk)
var buildFtrSpace = true;
// Learn a model that filters "relevant" documents (if learnSvmFilter is false, then the model is loaded from disk)
var learnSvmFilter = true;
// Learn a sentiment model (if learnSvmSentiment is false, then the model is loaded from disk)
var learnSvmSentiment = true;

// Load everything?
var justLoad = false;
if (justLoad) {
    buildFtrSpace = false;
    learnSvmFilter = false;
    learnSvmSentiment = false;
}
// The feature space provides the mapping from documents (tweets) to sparse vectors (provided by linear algebra module)
// Create or load feature space
var ftrSpace = analytics.newFeatureSpace([
	{ type: "text", source: "Tweets", field: "Text" },
]);
if (buildFtrSpace) {
    // Builds a new feature space
    ftrSpace.updateRecords(recSet);
    // Saves the feature space
    var fout = fs.openWrite("./sandbox/twitter/fs.dat");
    ftrSpace.save(fout);
    fout.close();
} else {
    // Load the feature space
    var fin = fs.openRead("./sandbox/twitter/fs.dat");
    ftrSpace = analytics.loadFeatureSpace(fin);
}


// Learn a model of relevant tweets 
if (learnSvmFilter) {
    // Constructs the active learner
    var AL = new analytics.activeLearner(ftrSpace, "Text", recSet, nPos, nNeg, relevantQuery);
    // Starts the active learner (use the keyword stop to quit)
    AL.selectQuestion();
    // Save the model
    var fout = fs.openWrite('./sandbox/twitter/svmFilter.bin');
    AL.saveSvmModel(fout);
    fout.close();
}
// Load the model from disk
var fin = fs.openRead("./sandbox/twitter/svmFilter.bin");
var svmFilter = analytics.loadSvmModel(fin);
// Filter relevant records: records are dropped if svmFilter predicts a v negative value (anonymous function)
recSet.filter(function (rec) { return svmFilter.predict(ftrSpace.ftrSpVec(rec)) > 0; });

// Learn a sentiment model 
if (learnSvmSentiment) {
    // Constructs the active learner
    var AL = new analytics.activeLearner(ftrSpace, "Text", recSet, nPos, nNeg, sentimentQuery);
    // Starts the active learner
    AL.selectQuestion();
    // Saves the sentiment model
    var fout = fs.openWrite('./sandbox/twitter/svmSentiment.bin');
    AL.saveSvmModel(fout);
    fout.close();
}
// Loads the sentiment model
var fin = fs.openRead('./sandbox/twitter/svmSentiment.bin');
var svmSentiment = analytics.loadSvmModel(fin);

// Classify the sentiment of the "relevant" tweets
for (var recN = 0; recN < recSet.length; recN++) {
    recSet[recN].Sentiment = svmSentiment.predict(ftrSpace.ftrSpVec(recSet[recN])) > 0 ? 1 : -1;
}

// Filter the record set of by time
// Clone the rec set two times
var recSet1 = recSet.clone();
var recSet2 = recSet.clone();
// Set the cutoff date
var tm = time.parse("2011-08-01T00:05:06");
// Get a record set with tweets older than tm
recSet1.filter(function (rec) { return rec.Date.timestamp < tm.timestamp })
// Get a record set with tweets newer than tm
recSet2.filter(function (rec) { return rec.Date.timestamp > tm.timestamp })
// Print the record set length
console.say("recSet1.length: " + recSet1.length + ", recSet2.length: " + recSet2.length);
// Build two communication graph snapshots based on the two record sets. Users represent graph nodes. A user "a" is linked to user "b" if "a" authored a tweet that contained the keyword @"b".
// Each node is assigned a sentiment (majority sentiment based on all the tweets authored by the node)
// Build the first graph and save it in DOT format (implemented in C++ as a qminer aggregate)
var u1 = recSet1.aggr({ name: "tgraph1", dotName: "tesi1", type: "twitterGraph", fName: "./sandbox/twitter/graph1.gv" });
// Build the second graph (based on the second record set) and filter the nodes that were not present in the first graph, finally save it in DOT format
var u2 = recSet2.aggr({ name: "tgraph2", dotName: "tesi2", type: "twitterGraph", fName: "./sandbox/twitter/graph2.gv", userVec: u1 });
// Start console
console.say("Interactive mode: use /stop to continue");
console.start();


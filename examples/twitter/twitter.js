// This example demonstrates text mining (feature vectors, active learning and classification)
// as well as record set filtering (based on time and classification results). It also builds
// communication graphs based on sets of twitter messages (twitter specific)

// Import libraries
var qm = require('qminer');
var analytics = qm.analytics;
var fs = qm.fs;

var base = new qm.Base({ mode: "createClean", schemaPath: "twitter.def" });

// Load tweets from a file (toy example)
// Set the filename
var tweetsFile = "./sandbox/twitter/toytweets.txt";
// Get the store
var Tweets = base.store("Tweets");
// Load tweets (each line is a json)
qm.load.jsonFile(Tweets, tweetsFile);
// Print number of records

console.log("number of records: " + Tweets.length);
// Select all tweets
var recSet = Tweets.allRecords;

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
var ftrSpace = new qm.FeatureSpace(base, [
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
    ftrSpace = new qm.FeatureSpace(base, fin);
}

// Learn a model of relevant tweets
if (learnSvmFilter) {
    // Constructs the active learner
    var al = new analytics.ActiveLearner(relevantQuery, recSet, undefined, ftrSpace,
        {nPos: nPos, nNeg: nNeg, textField: "Text"}
    );
    // Starts the active learner (use the keyword stop to quit)
    al.startLoop();
    // Save the model
    var fout = fs.openWrite('./sandbox/twitter/svmFilter.bin');
    al.saveSvmModel(fout);
    fout.close();
}

// Load the model from disk
var fin = fs.openRead("./sandbox/twitter/svmFilter.bin");
var svmFilter = new analytics.SVC(fin);
// Filter relevant records: records are dropped if svmFilter predicts a v negative value (anonymous function)
recSet.filter(function (rec) { return svmFilter.predict(ftrSpace.extractSparseVector(rec)) > 0; });

// Learn a sentiment model
if (learnSvmSentiment) {
    // Constructs the active learner
    var al = new analytics.ActiveLearner(sentimentQuery, recSet, undefined, ftrSpace,
        {nPos: nPos, nNeg: nNeg, textField: "Text"}
    );
    // Starts the active learner
    al.startLoop();
    // Saves the sentiment model
    var fout = fs.openWrite('./sandbox/twitter/svmSentiment.bin');
    al.saveSvmModel(fout);
    fout.close();
}
// Loads the sentiment model
var fin = fs.openRead('./sandbox/twitter/svmSentiment.bin');
var svmSentiment = new analytics.SVC(fin);

// Classify the sentiment of the "relevant" tweets
for (var recN = 0; recN < recSet.length; recN++) {
    recSet[recN].Sentiment = svmSentiment.predict(ftrSpace.extractSparseVector(recSet[recN])) > 0 ? 1 : -1;
}

// Filter the record set of by time
// Clone the rec set two times
var recSet1 = recSet.clone();
var recSet2 = recSet.clone();

// Set the cutoff date
var tm = new Date("2011-08-01T00:05:06");
// Get a record set with tweets older than tm
recSet1.filter(function (rec) { return rec.Date.getTime() < tm.getTime() })
// Get a record set with tweets newer than tm
recSet2.filter(function (rec) { return rec.Date.getTime() > tm.getTime() })

// Print the record set length
console.log("recSet1.length: " + recSet1.length + ", recSet2.length: " + recSet2.length);

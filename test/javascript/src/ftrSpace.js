var assert = require('assert.js');

console.say("FtrSpace", "Testing feature space serialization/deserilization/by value");

// only report failours
assert.silent = !process.isArg("-verbose");
// name of the debug process
assert.consoleTitle = "FtrSpace";

// create store
console.log("Size: " + process.sysStat.size);
for (var i = 1000; i < 100; i++) {
    var diff = qm.sysStat.size;
    qm.createStore({
        "name": "FtrSpaceTest" + i, 
        "fields": [ 
          { "name": "Value", "type": "float" },
          { "name": "Category", "type": "string" },
          { "name": "Categories", "type": "string_v" },
          { "name": "Text", "type": "string" }
        ], 
        "joins": [ ],
        "keys": [ ]
      });
    var diff = process.sysStat.size - diff;
    console.log("Size: " + process.sysStat.size + ", Diff: " + diff);
}

// prepare test set
qm.createStore({
    "name": "FtrSpaceTest", 
    "fields": [ 
      { "name": "Value", "type": "float" },
      { "name": "Category", "type": "string" },
      { "name": "Categories", "type": "string_v" },
      { "name": "Text", "type": "string" }
    ], 
    "joins": [ ],
    "keys": [ ]
  });
var Store = qm.store("FtrSpaceTest");
Store.add({Value: 1.0, Category: "a", Categories: ["a", "q"], Text: "Barclays dropped a bombshell on its investment bankers last week." });
Store.add({Value: 1.1, Category: "b", Categories: ["b", "w"], Text: "Amid a general retreat by banks from bond trading and other volatile business lines, Barclays was particularly aggressive." });
Store.add({Value: 1.2, Category: "c", Categories: ["c", "e"], Text: "In what CEO Antony Jenkins dubbed a “bold simplification,” Barclays will cut 7,000 jobs in its investment bank by 2016 and will trim the unit to 30% of the group’s risk-weighted assets." });
Store.add({Value: 1.3, Category: "a", Categories: ["a", "q"], Text: "The bank is relegating £400 billion ($676 billion) in assets to its “non-core” unit, effectively quarantining them from the rest of the business." });
Store.add({Value: 1.4, Category: "b", Categories: ["b", "w"], Text: "Just about every large lender these days has chucked a chunk of its toxic cast-offs into a so-called “bad bank,” but none with the same zeal as Barclays." });
Store.add({Value: 1.5, Category: "c", Categories: ["c", "e"], Text: "The last time we drew up the league table for bad banks, UBS was on top." });
Store.add({Value: 1.6, Category: "a", Categories: ["a", "q"], Text: "But Barclays has now taken the crown, with “non-core” assets accounting for nearly 30% of the bank’s total balance sheet." });
Store.add({Value: 1.7, Category: "b", Categories: ["b", "w"], Text: "Details about a bad bank are typically relegated deep in the depths of a bank’s financial report, while it prominently trumpets the brilliant “adjusted” results of its unsullied core business." });
Store.add({Value: 1.8, Category: "c", Categories: ["c", "e"], Text: "These assets still belong to the bank, and factor into its capital ratios and other important measures of financial soundness." });
Store.add({Value: 1.9, Category: "a", Categories: ["a", "q"], Text: "But because selling everything at once would produce a huge loss, carving out an internal bad bank is the next best option." });
Store.add({Value: 2.0, Category: "b", Categories: ["b", "w"], Text: "The Barbie doll is an icon that young girls have played with since 1959, when Barbie settled in as an American fixture in the lives of children, first in the United States and in more recent years, worldwide." });

// test feature space by creating and printing vectors
function testFtrSpace(ftrSpace) {
    for (var i = 0; i < Store.length; i++) {
        var rec = Store[i];
        var vec = ftrSpace.ftrVec(rec);
        //vec.print();
    }
}

function compareFtrSpace(ftrSpace1, ftrSpace2) {
    for (var i = 0; i < Store.length; i++) {
        var rec = Store[i];
        var vec1 = ftrSpace1.ftrVec(rec);
        var vec2 = ftrSpace2.ftrVec(rec);
        console.log("Diff: " + vec1.minus(vec2).norm());
    }
}

console.log("Prepare feature space");
var analytics = require("analytics.js");
var ftrSpace1 = analytics.newFeatureSpace([
    //{ type: "random", source: "FtrSpaceTest", seed: 1 },
    { type: "numeric", source: "FtrSpaceTest", field: "Value" },
    { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] },
    { type: "categorical", source: "FtrSpaceTest", field: "Category", hashDimension: 2 },
    { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] },
    { type: "multinomial", source: "FtrSpaceTest", field: "Categories", hashDimension: 4 },
    { type: "text", source: "FtrSpaceTest", field: "Text", hashDimension: 4, ngrams:[1,4] },
    { type: "pair", source: "FtrSpaceTest",
        first: { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] },
        second: { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] }
    }
]);
ftrSpace1.updateRecords(Store.recs);
console.log("Test new feature space");
testFtrSpace(ftrSpace1); 

console.log("Saving to disk");
var fout = fs.openWrite("./sandbox/ftrSpace/fs.dat");
ftrSpace1.save(fout);
fout.close();

console.log("Load from disk");
var fin = fs.openRead("./sandbox/ftrSpace/fs.dat");
var ftrSpace2 = analytics.loadFeatureSpace(fin);

console.log("Test loaded feature space");
testFtrSpace(ftrSpace2);

console.log("Compare output of feature spaces");
compareFtrSpace(ftrSpace1, ftrSpace2);


console.log("Test n-gram features");

// Unsorted array comparison
function arraysIdentical(a, b) {
    var i = a.length;
    if (i != b.length) return false;
    a.sort(); b.sort();
    while (i--) {
        if (a[i] !== b[i]) return false;
    }
    return true;
}

function sortNumber(a,b) {
    return a - b;
}

function printError(p, t) {
  if(!arraysIdentical(p, t)) {
    if(typeof t[0] == 'number') {
      console.log('numeric');
      p = p.sort(sortNumber);
      t = t.sort(sortNumber);
    } else {
      p = p.sort();
      t = t.sort();
    }
    console.log("predicted/true");
    console.log(p);
    console.log(p.length);
    console.log(t);
    console.log(t.length);
  }
}

var newText = "I like ngrams and tests";
var testRec = Store.newRec({Value: 2.0, Category: "b", Categories: ["b", "w"], Text: newText });
var ftrSpace1 = analytics.newFeatureSpace([
    //{ type: "random", source: "FtrSpaceTest", seed: 1 },
    { type: "text", source: "FtrSpaceTest", field: "Text", ngrams:[1,3], tokenizer:{type:'simple', stopwords: 'none'} },
]);
var trueAnswer = ['I', 'I LIKE', 'I LIKE NGRAMS', 'LIKE', 'LIKE NGRAMS', 'LIKE NGRAMS AND', 'NGRAMS', 'NGRAMS AND', 'NGRAMS AND TESTS', 'AND', 'AND TESTS', 'TESTS']; 
ftrSpace1.updateRecords(Store.recs);
ftrSpace1.updateRecord(testRec);
var vec = ftrSpace1.ftrSpVec(testRec);
var testAnswer = la.getSpFeatVecCols(vec, ftrSpace1);
assert.ok(arraysIdentical(testAnswer, trueAnswer), "ngrams 1,3");
printError(testAnswer, trueAnswer);

var ftrSpace2 = analytics.newFeatureSpace([
    //{ type: "random", source: "FtrSpaceTest", seed: 1 },
    { type: "text", source: "FtrSpaceTest", field: "Text", tokenizer:{type:'simple', stopwords: 'none'} },
]);
trueAnswer = ['I', 'LIKE', 'NGRAMS', 'AND', 'TESTS'];
ftrSpace2.updateRecords(Store.recs);
ftrSpace2.updateRecord(testRec);
vec = ftrSpace2.ftrSpVec(testRec);
testAnswer = la.getSpFeatVecCols(vec, ftrSpace2);
assert.ok(arraysIdentical(testAnswer, trueAnswer), "ngrams default (1)");
printError(testAnswer, trueAnswer);

var ftrSpace3 = analytics.newFeatureSpace([
    //{ type: "random", source: "FtrSpaceTest", seed: 1 },
    { type: "text", source: "FtrSpaceTest", field: "Text", tokenizer:{type:'simple', stopwords: 'none'}, ngrams:[2,4]},
]);
trueAnswer = ['I LIKE', 'I LIKE NGRAMS', 'I LIKE NGRAMS AND', 'LIKE NGRAMS', 'LIKE NGRAMS AND', 'LIKE NGRAMS AND TESTS', 'NGRAMS AND', 'NGRAMS AND TESTS', 'AND TESTS'];
ftrSpace3.updateRecords(Store.recs);
ftrSpace3.updateRecord(testRec);
vec = ftrSpace3.ftrSpVec(testRec);
testAnswer = la.getSpFeatVecCols(vec, ftrSpace3);
assert.ok(arraysIdentical(testAnswer, trueAnswer), "ngrams 2, 4");
printError(testAnswer, trueAnswer);

var ftrSpace4 = analytics.newFeatureSpace([
    //{ type: "random", source: "FtrSpaceTest", seed: 1 },
    { type: "text", source: "FtrSpaceTest", field: "Text", tokenizer:{type:'simple', stopwords: 'none'}, ngrams:[2,4], hashDimension: 320},
]);
trueAnswer = [ 239, 183, 57, 274, 266, 153, 257, 200, 8 ];
trueAnswer = trueAnswer.map(function(n) { return n.toString(); });
ftrSpace4.updateRecords(Store.recs);
ftrSpace4.updateRecord(testRec);
vec = ftrSpace4.ftrSpVec(testRec);
testAnswer = la.getSpFeatVecCols(vec, ftrSpace4);
assert.ok(arraysIdentical(testAnswer, trueAnswer), "ngrams 2, 4 with hashing");
printError(testAnswer, trueAnswer);

var ftrSpace5 = analytics.newFeatureSpace([
    //{ type: "random", source: "FtrSpaceTest", seed: 1 },
    { type: "text", source: "FtrSpaceTest", field: "Text", tokenizer:{type:'simple', stopwords: 'none'},
     ngrams:[2,4], hashDimension: 320, hashTable: true},
]);
trueAnswer = ["OTHER VOLATILE", "UNIT TO", "DEPTHS OF", "WHEN BARBIE SETTLED", "I LIKE"];

ftrSpace5.updateRecords(Store.recs);
ftrSpace5.updateRecord(testRec);
testAnswer = ftrSpace5.getFtr(239).split(",");
assert.ok(arraysIdentical(testAnswer, trueAnswer), "hashing with tables");
printError(testAnswer, trueAnswer);
eval(breakpoint);



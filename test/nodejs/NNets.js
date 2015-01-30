var fs = require("../../build/Debug/fs.node");
var assert = require('assert');
var analytics = require('../../build/Debug/analytics.node')
var qm = require('../../build/Debug/qm.node');
var linalg = require("../../build/Debug/la.node")

qm.config('qm.conf', true, 8080, 1024);
// add store.addTrigger method
console.log("NNets", "Starting test");

console.log(analytics)
var NN = new analytics.NNet({"layout":[2,4,1]});
var lastInVec = [];
var lastTargVec = [];
// create a loop for learning the net
for(var i = 0; i < 20; ++i){
    // get two random numbers 0 or 1, this is the input data
    var in1 = Math.round(Math.random())
    var in2 = Math.round(Math.random())
    // perform an xor on the variables, this will be the target value
    var out1 = 0
    if(!in1 ^ !in2)
        out1 = 1
    // for learning the net the data must be in vector form, so we create vectors.
    // dimensions of the vectors should match the dimensions of the input and output layers.
    var inVec = linalg.Vector([in1, in2])
    var outVec = linalg.Vector([out1])
    console.log("In 1: " + in1 + " In 2: " + in2)
    console.log("Target: " + out1)
    // first we predict based on the inputs (feed-forward)
    var predictions = NN.predict(inVec)
    console.log('----------------------')
    console.log("Result: " + predictions[0])
    console.log("Diff: " + (out1 - predictions[0]))
    // then we learn the net with inputs and expected outputs (back propagation)
    NN.fit(inVec,outVec);
    lastInVec = inVec;
    lastTargVec = outVec;
    console.log("In vector: " + lastInVec);
    console.log("Prediction: " + predictions)

}

//save
console.log('---------------------- SAVING ------------------------')
var fout = fs.openWrite("NN.bin");
NN.save(fout);
fout.close();
console.log('---------------------- SAVED ------------------------')

predictions2 = NN.predict(lastInVec)
console.log("BeforeSave In vector: " + lastInVec);
console.log("BeforeSave Prediction: " + predictions2)

// load
console.log('---------------------- LOADING ------------------------')
var fin = fs.openRead("NN.bin");
var NNIn = new analytics.NNet(fin);
console.log('---------------------- LOADED ------------------------')
console.log('InVec length: ' + lastInVec.length)

predictions2 = NNIn.predict(lastInVec)
console.log("AfterSave In vector: " + lastInVec);
console.log("AfterSave Prediction: " + predictions2)
NNIn.fit(lastInVec, lastTargVec)
predictions2 = NNIn.predict(lastInVec)
console.log("AfterSave and fit In vector: " + lastInVec);
console.log("AfterSave and fit Prediction: " + predictions2)

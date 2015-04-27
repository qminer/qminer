/**
 * Copyright (c) 2015, Quintelligence d.o.o.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 */
 
console.log(__filename)
var qm = require('qminer');
var fs = qm.fs;
var assert = require('assert');
var analytics = qm.analytics;
var linalg = qm.la;


describe('NNets test, old', function () {
	it('should survive', function () {
	

console.log("NNets", "Starting test");

var NN = new analytics.NNet({"layout":[2,4,1]});

var lastInVec = [];
var lastTargVec = [];
// create a loop for learning the net
for(var i = 0; i < 2000; ++i){
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
    //console.log("In 1: " + in1 + " In 2: " + in2)
    //console.log("Target: " + out1)
    // first we predict based on the inputs (feed-forward)
    var predictions = NN.predict(inVec)
    var error = Math.abs(predictions[0] - outVec[0]);
    // then we learn the net with inputs and expected outputs (back propagation)
    NN.fit(inVec,outVec);
    lastInVec = inVec;
    lastTargVec = outVec;
}

var testIn = linalg.Vector([0, 0])
var testOut = linalg.Vector([0])
error = Math.abs(NN.predict(testIn)[0] - testOut[0])

console.log("Is error small? : " + (error < 0.3))
console.log("Error : " + error)
//assert.equal(error < 0.3, true);

var testIn = linalg.Vector([0, 1])
var testOut = linalg.Vector([1])
error = Math.abs(NN.predict(testIn)[0] - testOut[0])

console.log("Is error small? : " + (error < 0.3))
console.log("Error : " + error)
//assert.equal(error < 0.3, true);

var testIn = linalg.Vector([1, 0])
var testOut = linalg.Vector([1])
error = Math.abs(NN.predict(testIn)[0] - testOut[0])

console.log("Is error small? : " + (error < 0.3))
console.log("Error : " + error)
//assert.equal(error < 0.3, true);

var testIn = linalg.Vector([1, 1])
var testOut = linalg.Vector([0])
error = Math.abs(NN.predict(testIn)[0] - testOut[0])

console.log("Is error small? : " + (error < 0.3))
console.log("Error : " + error)
//assert.equal(error < 0.3, true);

//save
var fout = fs.openWrite("NN.bin");
NN.save(fout);
fout.close();

predictions_beforeSave = NN.predict(lastInVec)
console.log("BeforeSave In vector: " + lastInVec);
console.log("BeforeSave Prediction: " + predictions_beforeSave)

// load
var fin = fs.openRead("NN.bin");
var NNIn = new analytics.NNet(fin);

predictions_afterSave = NNIn.predict(lastInVec)
console.log("AfterSave In vector: " + lastInVec);
console.log("AfterSave Prediction: " + predictions_afterSave)

//check if we survive the fitting
NNIn.fit(lastInVec, lastTargVec)
predictions_afterSaveAndFit = NNIn.predict(lastInVec)
console.log("AfterSave and fit In vector: " + lastInVec);
console.log("AfterSave and fit Prediction: " + predictions_afterSaveAndFit)

// check if nnet was saved and loaded correctly
diff = predictions_afterSave.minus(predictions_beforeSave);

assert.equal(diff.norm(), 0);
})});
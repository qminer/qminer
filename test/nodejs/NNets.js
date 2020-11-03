/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

// console.log(__filename)
var qm = require('../../index.js');
var fs = qm.fs;
var assert = require('assert');
var analytics = qm.analytics;
var linalg = qm.la;

var verbose = false;

describe('NNets test, old', function () {
    it('should survive', function () {

        if (verbose) {
            console.log("NNets", "Starting test");
        }
        var NN = new analytics.NNet({ "layout": [2, 4, 1] });

        var lastInVec = [];
        var lastTargVec = [];
        // create a loop for learning the net
        for (var i = 0; i < 2000; ++i) {
            // get two random numbers 0 or 1, this is the input data
            var in1 = Math.round(Math.random())
            var in2 = Math.round(Math.random())
            // perform an xor on the variables, this will be the target value
            var out1 = 0
            if (!in1 ^ !in2)
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
            NN.fit(inVec, outVec);
            lastInVec = inVec;
            lastTargVec = outVec;
        }

        var testIn = linalg.Vector([0, 0])
        var testOut = linalg.Vector([0])
        error = Math.abs(NN.predict(testIn)[0] - testOut[0])

        if (verbose) {
            console.log("Is error small? : " + (error < 0.3))
            console.log("Error : " + error)
            //assert.equal(error < 0.3, true);
        }
        var testIn = linalg.Vector([0, 1])
        var testOut = linalg.Vector([1])
        error = Math.abs(NN.predict(testIn)[0] - testOut[0])

        if (verbose) {
            console.log("Is error small? : " + (error < 0.3))
            console.log("Error : " + error)
            //assert.equal(error < 0.3, true);
        }

        var testIn = linalg.Vector([1, 0])
        var testOut = linalg.Vector([1])
        error = Math.abs(NN.predict(testIn)[0] - testOut[0])

        if (verbose) {
            console.log("Is error small? : " + (error < 0.3))
            console.log("Error : " + error)
            //assert.equal(error < 0.3, true);
        }

        var testIn = linalg.Vector([1, 1])
        var testOut = linalg.Vector([0])
        error = Math.abs(NN.predict(testIn)[0] - testOut[0])

        if (verbose) {
            console.log("Is error small? : " + (error < 0.3))
            console.log("Error : " + error)
            //assert.equal(error < 0.3, true);
        }

        //save
        var fout = fs.openWrite("NN.bin");
        NN.save(fout);
        fout.close();

        predictions_beforeSave = NN.predict(lastInVec)
        if (verbose) {
            console.log("BeforeSave In vector: " + lastInVec);
            console.log("BeforeSave Prediction: " + predictions_beforeSave)
        }
        // load
        var fin = fs.openRead("NN.bin");
        var NNIn = new analytics.NNet(fin);

        predictions_afterSave = NNIn.predict(lastInVec)
        if (verbose) {
            console.log("AfterSave In vector: " + lastInVec);
            console.log("AfterSave Prediction: " + predictions_afterSave)
        }
        //check if we survive the fitting
        NNIn.fit(lastInVec, lastTargVec)
        predictions_afterSaveAndFit = NNIn.predict(lastInVec)
        if (verbose) {
            console.log("AfterSave and fit In vector: " + lastInVec);
            console.log("AfterSave and fit Prediction: " + predictions_afterSaveAndFit)
        }
        // check if nnet was saved and loaded correctly
        diff = predictions_afterSave.minus(predictions_beforeSave);

        assert.equal(diff.norm(), 0);
    })
});
